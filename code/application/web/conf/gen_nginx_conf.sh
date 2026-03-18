#!/bin/sh
#
# Generate nginx.conf from network.json configuration
#

CONFIG_DIR="/etc/ipcamera/configs/config.factory.d"
NETWORK_JSON="$CONFIG_DIR/network.json"
NGINX_CONF="/usr/conf/nginx.conf"
NGINX_TEMPLATE="/usr/conf/nginx.conf.template"

# Check if jq is available for JSON parsing
if ! command -v jq >/dev/null 2>&1; then
    echo "Warning: jq not found, using default nginx config"
    exit 0
fi

# Check if network.json exists
if [ ! -f "$NETWORK_JSON" ]; then
    echo "Warning: $NETWORK_JSON not found, using default nginx config"
    exit 0
fi

# Parse configuration from network.json
HTTP_ENABLED=$(jq -r '.network.http.enabled // true' "$NETWORK_JSON")
HTTP_PORT=$(jq -r '.network.http.port // 80' "$NETWORK_JSON")
HTTPS_ENABLED=$(jq -r '.network.https.enabled // true' "$NETWORK_JSON")
HTTPS_PORT=$(jq -r '.network.https.port // 443' "$NETWORK_JSON")
WEB_ROOT=$(jq -r '.network.http.web_root // "/var/www/"' "$NETWORK_JSON")
SSL_CERT=$(jq -r '.network.https.ssl_certificate // "/etc/nginx/ssl/Server.crt"' "$NETWORK_JSON")
SSL_KEY=$(jq -r '.network.https.ssl_certificate_key // "/etc/nginx/ssl/Server.key"' "$NETWORK_JSON")
SSL_DIR=$(jq -r '.network.https.ssl_certs_dir // "/etc/nginx/ssl"' "$NETWORK_JSON")

# Generate nginx configuration
cat > "$NGINX_CONF" << EOF
user nobody;
worker_processes 1;
worker_rlimit_nofile 512;
error_log /var/log/nginx/error.log error;
pid /var/run/nginx.pid;

events {
    worker_connections 128;
    use epoll;
}

http {
    include mime.types;
    default_type application/octet-stream;

    # Logging - minimal for embedded
    access_log off;
    error_log /var/log/nginx/error.log error;

    # Performance - optimized for low memory
    sendfile on;
    tcp_nopush on;
    tcp_nodelay on;
    keepalive_timeout 30;
    keepalive_requests 50;
    reset_timedout_connection on;
    client_body_timeout 10;
    send_timeout 10;

    # Buffer sizes - reduced for 512MB RAM
    client_body_buffer_size 16k;
    client_max_body_size 200m;  # Generous limit for firmware uploads
    client_header_buffer_size 1k;
    large_client_header_buffers 2 4k;
    client_body_temp_path /mnt/app/nginx_temp 1 2;

    # Gzip - disabled to save CPU/RAM
    gzip off;

    # Essential security headers
    add_header X-Frame-Options "SAMEORIGIN" always;
    add_header X-Content-Type-Options "nosniff" always;
    add_header X-XSS-Protection "1; mode=block" always;
    add_header Referrer-Policy "strict-origin-when-cross-origin" always;

    # Hide nginx version
    server_tokens off;

    # Rate limiting zones - optimized for embedded
    limit_req_zone \$binary_remote_addr zone=api_limit:512k rate=50r/s;
    limit_conn_zone \$binary_remote_addr zone=conn_limit:256k;

    # Upstream for CivetWeb Unix socket
    upstream civetweb_backend {
        server unix:/var/run/ipcam_api.sock;
    }
EOF

# Add HTTP server block if enabled
if [ "$HTTP_ENABLED" = "true" ]; then
    cat >> "$NGINX_CONF" << EOF

    # HTTP server - redirect to HTTPS
    server {
        listen $HTTP_PORT;
        server_name _;
EOF

    # If HTTPS is enabled, redirect HTTP to HTTPS
    if [ "$HTTPS_ENABLED" = "true" ]; then
        cat >> "$NGINX_CONF" << EOF
        
        # Allow snapshot over HTTP (camera functionality)
        location /snapshot {
            root /tmp;
            add_header Content-Disposition "inline";
            try_files /test_jpeg0.jpeg =404;
        }
        
        # Redirect everything else to HTTPS
        location / {
            return 301 https://\$host\$request_uri;
        }
EOF
    else
        # Otherwise serve content directly (HTTP-only mode)
        cat >> "$NGINX_CONF" << EOF
        
        # API endpoints - reverse proxy to CivetWeb Unix socket
        location /api/ {
            limit_req zone=api_limit burst=50 nodelay;
            limit_conn conn_limit 25;
            
            proxy_pass http://civetweb_backend;
            proxy_http_version 1.1;
            proxy_buffering off;
            proxy_request_buffering off;
            proxy_set_header Host \$host;
            proxy_set_header X-Real-IP \$remote_addr;
            proxy_set_header X-Forwarded-For \$proxy_add_x_forwarded_for;
            proxy_set_header X-Forwarded-Proto \$scheme;
            
            proxy_connect_timeout 30s;
            proxy_send_timeout 120s;
            proxy_read_timeout 120s;
        }

        # Firmware upload - dynamic timeout
        location /api/firmware/upload {
            limit_conn conn_limit 1;
            
            proxy_pass http://civetweb_backend;
            proxy_http_version 1.1;
            proxy_buffering off;
            proxy_request_buffering off;
            proxy_set_header Host \$host;
            proxy_set_header X-Real-IP \$remote_addr;
            proxy_set_header X-Forwarded-For \$proxy_add_x_forwarded_for;
            proxy_set_header X-Forwarded-Proto \$scheme;
            
            proxy_connect_timeout 60s;
            proxy_send_timeout 0;
            proxy_read_timeout 0;
        }

        # Static files
        location / {
            root $WEB_ROOT;
            index index.html index.htm index.csr.html;
            try_files \$uri \$uri/ /index.csr.html;
            
            location ~* \\.(jpg|jpeg|png|gif|ico|css|js|svg|woff|woff2|ttf|eot)$ {
                expires 7d;
                add_header Cache-Control "public";
            }
        }

        # BMP images from SD card
        location ~* \.bmp$ {
            root /mnt/sd;
            add_header Content-Disposition "inline";
            add_header Cache-Control "no-cache";
            expires 0;
        }

        # Snapshot endpoint
        location /snapshot {
            root /tmp;
            add_header Content-Disposition "inline";
            add_header Cache-Control "no-cache";
            try_files /test_jpeg0.jpeg =404;
        }

        # Security - block hidden files
        location ~ /\\. {
            deny all;
            access_log off;
            log_not_found off;
        }

        # Custom 404 fallback to root
        error_page 404 /;
EOF
    fi

    cat >> "$NGINX_CONF" << EOF
    }
EOF
fi

# Add HTTPS server block if enabled
if [ "$HTTPS_ENABLED" = "true" ]; then
    # Ensure SSL certs directory exists
    mkdir -p "$SSL_DIR"
    
    cat >> "$NGINX_CONF" << EOF

    # HTTPS server
    server {
        listen $HTTPS_PORT ssl;
        http2 on;
        server_name _;

        # SSL configuration - optimized for embedded with HTTP/2
        ssl_certificate $SSL_CERT;
        ssl_certificate_key $SSL_KEY;
        ssl_protocols TLSv1.2 TLSv1.3;
        ssl_prefer_server_ciphers off;
        ssl_ciphers 'ECDHE-RSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384';
        ssl_session_cache shared:SSL:512k;
        ssl_session_timeout 5m;
        ssl_session_tickets off;

        # HTTP/2 settings for embedded
        http2_max_concurrent_streams 32;
        client_header_timeout 30s;

        # Root and index
        root $WEB_ROOT;
        index index.html index.htm index.csr.html;

        # API endpoints - reverse proxy to CivetWeb Unix socket
        location /api/ {
            # Rate limiting for API protection (permissive)
            limit_req zone=api_limit burst=50 nodelay;
            limit_conn conn_limit 25;
            
            proxy_pass http://civetweb_backend;
            proxy_http_version 1.1;
            proxy_buffering off;
            proxy_request_buffering off;
            proxy_set_header Host \$host;
            proxy_set_header X-Real-IP \$remote_addr;
            proxy_set_header X-Forwarded-For \$proxy_add_x_forwarded_for;
            proxy_set_header X-Forwarded-Proto \$scheme;
            
            # Default timeouts for regular API calls
            proxy_connect_timeout 30s;
            proxy_send_timeout 120s;
            proxy_read_timeout 120s;
        }

        # Firmware upload - dynamic timeout based on upload speed
        location /api/firmware/upload {
            limit_conn conn_limit 1;
            
            proxy_pass http://civetweb_backend;
            proxy_http_version 1.1;
            proxy_buffering off;
            proxy_request_buffering off;
            proxy_set_header Host \$host;
            proxy_set_header X-Real-IP \$remote_addr;
            proxy_set_header X-Forwarded-For \$proxy_add_x_forwarded_for;
            proxy_set_header X-Forwarded-Proto \$scheme;
            
            # Extended for large uploads - dynamic based on data flow
            proxy_connect_timeout 60s;
            proxy_send_timeout 0;
            proxy_read_timeout 0;
        }

        # Static files
        location / {
            try_files \$uri \$uri/ /index.csr.html;
            
            # Cache static assets - shorter for embedded
            location ~* \\.(jpg|jpeg|png|gif|ico|css|js|svg|woff|woff2|ttf|eot)$ {
                expires 7d;
                add_header Cache-Control "public";
            }
        }

        # BMP images from SD card
        location ~* \.bmp$ {
            root /mnt/sd;
            add_header Content-Disposition "inline";
            add_header Cache-Control "no-cache";
            expires 0;
        }

        # Snapshot endpoint
        location /snapshot {
            root /tmp;
            add_header Content-Disposition "inline";
            add_header Cache-Control "no-cache";
            try_files /test_jpeg0.jpeg =404;
        }

        # Security - block hidden files
        location ~ /\\. {
            deny all;
            access_log off;
            log_not_found off;
        }

        # Custom 404 fallback to root
        error_page 404 /;
    }
EOF
fi

# Close HTTP block
cat >> "$NGINX_CONF" << EOF
}
EOF

echo "nginx.conf generated successfully from $NETWORK_JSON"
echo "HTTP: enabled=$HTTP_ENABLED, port=$HTTP_PORT"
echo "HTTPS: enabled=$HTTPS_ENABLED, port=$HTTPS_PORT, certs_dir=$SSL_DIR"
