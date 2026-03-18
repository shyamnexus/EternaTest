# Web Folder Information

### Author Manas Chachra (H582143)

The `web` folder (`ns02302_linux_sdk/code/application/web`) contains all resources and configuration files required for the web server (nginx) and web application:

- **certs/**: Contains SSL certificates used by nginx for HTTPS connections.
- **conf/**: Holds nginx configuration files, including settings for `fcgiwrapper` and other related services.
- **init.d/**: Contains initialization scripts to start nginx, fcgiwrapper, and to set the necessary permissions for the `www-data` user on required folders.
- **www/**: Stores the Angular build files that are served as the frontend of the web application.
- **src/**: Contains CGI source code, which is compiled and used by the web server for backend processing.

The `www-data` user, which is used by nginx and fcgiwrap, is created and configured via the Makefile in this directory. This user is granted the necessary permissions to access and serve web content securely.
