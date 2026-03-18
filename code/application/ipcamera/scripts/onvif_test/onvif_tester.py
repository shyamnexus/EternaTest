#!/usr/bin/env python3
"""
ONVIF Compliance Tester for IP Cameras

This tool tests various ONVIF services and profiles to verify compliance.
Usage: python onvif_tester.py <camera_ip> [port] [username] [password]
"""

import sys
import json
from datetime import datetime
from onvif import ONVIFCamera
import zeep

# Custom settings to handle ONVIF quirks
def zeep_pythonvalue(self, xmlvalue):
    return xmlvalue

zeep.xsd.simple.AnySimpleType.pythonvalue = zeep_pythonvalue


class ONVIFTester:
    """ONVIF Compliance Testing Tool"""
    
    def __init__(self, ip, port=80, username='admin', password='admin'):
        self.ip = ip
        self.port = port
        self.username = username
        self.password = password
        self.camera = None
        self.results = {
            'device_info': {},
            'services': {},
            'profiles': [],
            'tests': {},
            'summary': {
                'passed': 0,
                'failed': 0,
                'warnings': 0
            }
        }
        
    def log(self, message, level='INFO'):
        """Print log message with timestamp"""
        timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        colors = {
            'INFO': '\033[94m',
            'PASS': '\033[92m',
            'FAIL': '\033[91m',
            'WARN': '\033[93m',
            'RESET': '\033[0m'
        }
        color = colors.get(level, colors['INFO'])
        print(f"{color}[{timestamp}] [{level}] {message}{colors['RESET']}")
        
    def record_test(self, test_name, passed, message='', warning=False):
        """Record test result"""
        self.results['tests'][test_name] = {
            'passed': passed,
            'message': message,
            'warning': warning
        }
        if passed and not warning:
            self.results['summary']['passed'] += 1
            self.log(f"✓ {test_name}: {message}", 'PASS')
        elif warning:
            self.results['summary']['warnings'] += 1
            self.log(f"⚠ {test_name}: {message}", 'WARN')
        else:
            self.results['summary']['failed'] += 1
            self.log(f"✗ {test_name}: {message}", 'FAIL')
            
    def connect(self):
        """Connect to the ONVIF camera"""
        self.log(f"Connecting to camera at {self.ip}:{self.port}...")
        try:
            self.camera = ONVIFCamera(
                self.ip,
                self.port,
                self.username,
                self.password
            )
            self.record_test('Connection', True, f"Successfully connected to {self.ip}:{self.port}")
            return True
        except Exception as e:
            self.record_test('Connection', False, f"Failed to connect: {str(e)}")
            return False
            
    def test_device_service(self):
        """Test Device Management Service"""
        self.log("Testing Device Management Service...")
        
        try:
            # Get Device Information
            device_service = self.camera.devicemgmt
            device_info = device_service.GetDeviceInformation()
            
            self.results['device_info'] = {
                'manufacturer': str(device_info.Manufacturer) if hasattr(device_info, 'Manufacturer') else 'N/A',
                'model': str(device_info.Model) if hasattr(device_info, 'Model') else 'N/A',
                'firmware_version': str(device_info.FirmwareVersion) if hasattr(device_info, 'FirmwareVersion') else 'N/A',
                'serial_number': str(device_info.SerialNumber) if hasattr(device_info, 'SerialNumber') else 'N/A',
                'hardware_id': str(device_info.HardwareId) if hasattr(device_info, 'HardwareId') else 'N/A'
            }
            
            self.record_test('GetDeviceInformation', True, 
                           f"Manufacturer: {self.results['device_info']['manufacturer']}, "
                           f"Model: {self.results['device_info']['model']}")
        except Exception as e:
            self.record_test('GetDeviceInformation', False, str(e))
            
        # Test GetCapabilities
        try:
            capabilities = device_service.GetCapabilities({'Category': 'All'})
            self.record_test('GetCapabilities', True, "Retrieved device capabilities")
            
            # Check for various services
            if hasattr(capabilities, 'Media'):
                self.results['services']['media'] = True
                self.log("  - Media service available")
            if hasattr(capabilities, 'PTZ'):
                self.results['services']['ptz'] = True
                self.log("  - PTZ service available")
            if hasattr(capabilities, 'Imaging'):
                self.results['services']['imaging'] = True
                self.log("  - Imaging service available")
            if hasattr(capabilities, 'Events'):
                self.results['services']['events'] = True
                self.log("  - Events service available")
            if hasattr(capabilities, 'Analytics'):
                self.results['services']['analytics'] = True
                self.log("  - Analytics service available")
                
        except Exception as e:
            self.record_test('GetCapabilities', False, str(e))
            
        # Test GetSystemDateAndTime
        try:
            date_time = device_service.GetSystemDateAndTime()
            self.record_test('GetSystemDateAndTime', True, "Retrieved system date and time")
        except Exception as e:
            self.record_test('GetSystemDateAndTime', False, str(e))
            
        # Test GetScopes
        try:
            scopes = device_service.GetScopes()
            self.record_test('GetScopes', True, f"Retrieved {len(scopes)} scopes")
        except Exception as e:
            self.record_test('GetScopes', False, str(e))
            
        # Test GetServices
        try:
            services = device_service.GetServices({'IncludeCapability': False})
            service_list = [str(s.Namespace).split('/')[-1] for s in services if hasattr(s, 'Namespace')]
            self.record_test('GetServices', True, f"Services: {', '.join(service_list)}")
        except Exception as e:
            self.record_test('GetServices', False, str(e))
            
        # Test GetHostname
        try:
            hostname = device_service.GetHostname()
            self.record_test('GetHostname', True, f"Hostname: {hostname.Name if hasattr(hostname, 'Name') else 'N/A'}")
        except Exception as e:
            self.record_test('GetHostname', False, str(e))
            
        # Test GetNetworkInterfaces
        try:
            interfaces = device_service.GetNetworkInterfaces()
            self.record_test('GetNetworkInterfaces', True, f"Found {len(interfaces)} network interface(s)")
        except Exception as e:
            self.record_test('GetNetworkInterfaces', False, str(e))
            
    def test_media_service(self):
        """Test Media Service"""
        self.log("Testing Media Service...")
        
        try:
            media_service = self.camera.create_media_service()
            
            # Get Profiles
            try:
                profiles = media_service.GetProfiles()
                self.results['profiles'] = []
                
                for profile in profiles:
                    profile_info = {
                        'name': str(profile.Name) if hasattr(profile, 'Name') else 'N/A',
                        'token': str(profile.token) if hasattr(profile, 'token') else 'N/A'
                    }
                    
                    # Check video encoder configuration
                    if hasattr(profile, 'VideoEncoderConfiguration') and profile.VideoEncoderConfiguration:
                        vec = profile.VideoEncoderConfiguration
                        profile_info['video_encoding'] = str(vec.Encoding) if hasattr(vec, 'Encoding') else 'N/A'
                        if hasattr(vec, 'Resolution'):
                            profile_info['resolution'] = f"{vec.Resolution.Width}x{vec.Resolution.Height}"
                            
                    self.results['profiles'].append(profile_info)
                    
                self.record_test('GetProfiles', True, f"Found {len(profiles)} media profile(s)")
                
                for p in self.results['profiles']:
                    self.log(f"  - Profile: {p['name']} ({p.get('video_encoding', 'N/A')}, {p.get('resolution', 'N/A')})")
                    
            except Exception as e:
                self.record_test('GetProfiles', False, str(e))
                
            # Get Video Sources
            try:
                video_sources = media_service.GetVideoSources()
                self.record_test('GetVideoSources', True, f"Found {len(video_sources)} video source(s)")
            except Exception as e:
                self.record_test('GetVideoSources', False, str(e))
                
            # Get Audio Sources
            try:
                audio_sources = media_service.GetAudioSources()
                self.record_test('GetAudioSources', True, f"Found {len(audio_sources)} audio source(s)")
            except Exception as e:
                self.record_test('GetAudioSources', True, "No audio sources (may be expected)", warning=True)
                
            # Get Stream URI for each profile
            if self.results['profiles']:
                for profile in self.results['profiles']:
                    try:
                        stream_setup = {
                            'Stream': 'RTP-Unicast',
                            'Transport': {'Protocol': 'RTSP'}
                        }
                        uri = media_service.GetStreamUri(stream_setup, profile['token'])
                        profile['stream_uri'] = str(uri.Uri) if hasattr(uri, 'Uri') else 'N/A'
                        self.record_test(f"GetStreamUri ({profile['name']})", True, 
                                       f"URI: {profile['stream_uri']}")
                    except Exception as e:
                        self.record_test(f"GetStreamUri ({profile['name']})", False, str(e))
                        
            # Get Snapshot URI for each profile
            if self.results['profiles']:
                for profile in self.results['profiles']:
                    try:
                        uri = media_service.GetSnapshotUri(profile['token'])
                        profile['snapshot_uri'] = str(uri.Uri) if hasattr(uri, 'Uri') else 'N/A'
                        self.record_test(f"GetSnapshotUri ({profile['name']})", True,
                                       f"URI: {profile['snapshot_uri']}")
                    except Exception as e:
                        self.record_test(f"GetSnapshotUri ({profile['name']})", False, str(e))
                        
            # Get Video Encoder Configurations
            try:
                encoders = media_service.GetVideoEncoderConfigurations()
                self.record_test('GetVideoEncoderConfigurations', True, 
                               f"Found {len(encoders)} video encoder configuration(s)")
            except Exception as e:
                self.record_test('GetVideoEncoderConfigurations', False, str(e))
                
        except Exception as e:
            self.record_test('MediaService', False, f"Failed to create media service: {str(e)}")
            
    def test_ptz_service(self):
        """Test PTZ Service"""
        if not self.results['services'].get('ptz'):
            self.log("PTZ service not available, skipping...")
            return
            
        self.log("Testing PTZ Service...")
        
        try:
            ptz_service = self.camera.create_ptz_service()
            
            # Get PTZ Configurations
            try:
                configs = ptz_service.GetConfigurations()
                self.record_test('GetPTZConfigurations', True, 
                               f"Found {len(configs)} PTZ configuration(s)")
            except Exception as e:
                self.record_test('GetPTZConfigurations', False, str(e))
                
            # Get PTZ Nodes
            try:
                nodes = ptz_service.GetNodes()
                self.record_test('GetPTZNodes', True, f"Found {len(nodes)} PTZ node(s)")
            except Exception as e:
                self.record_test('GetPTZNodes', False, str(e))
                
            # Get PTZ Status for first profile
            if self.results['profiles']:
                try:
                    status = ptz_service.GetStatus({'ProfileToken': self.results['profiles'][0]['token']})
                    self.record_test('GetPTZStatus', True, "Retrieved PTZ status")
                except Exception as e:
                    self.record_test('GetPTZStatus', False, str(e))
                    
        except Exception as e:
            self.record_test('PTZService', False, f"Failed to create PTZ service: {str(e)}")
            
    def test_imaging_service(self):
        """Test Imaging Service"""
        if not self.results['services'].get('imaging'):
            self.log("Imaging service not available, skipping...")
            return
            
        self.log("Testing Imaging Service...")
        
        try:
            imaging_service = self.camera.create_imaging_service()
            
            # Get Video Sources for imaging
            try:
                media_service = self.camera.create_media_service()
                video_sources = media_service.GetVideoSources()
                
                if video_sources:
                    source_token = video_sources[0].token
                    
                    # Get Imaging Settings
                    try:
                        settings = imaging_service.GetImagingSettings({'VideoSourceToken': source_token})
                        self.record_test('GetImagingSettings', True, "Retrieved imaging settings")
                        
                        # Log some imaging parameters
                        if hasattr(settings, 'Brightness'):
                            self.log(f"  - Brightness: {settings.Brightness}")
                        if hasattr(settings, 'Contrast'):
                            self.log(f"  - Contrast: {settings.Contrast}")
                        if hasattr(settings, 'Saturation'):
                            self.log(f"  - Saturation: {settings.Saturation}")
                            
                    except Exception as e:
                        self.record_test('GetImagingSettings', False, str(e))
                        
                    # Get Imaging Options
                    try:
                        options = imaging_service.GetOptions({'VideoSourceToken': source_token})
                        self.record_test('GetImagingOptions', True, "Retrieved imaging options")
                    except Exception as e:
                        self.record_test('GetImagingOptions', False, str(e))
                        
            except Exception as e:
                self.record_test('ImagingService', False, str(e))
                
        except Exception as e:
            self.record_test('ImagingService', False, f"Failed to create imaging service: {str(e)}")
            
    def test_events_service(self):
        """Test Events Service"""
        if not self.results['services'].get('events'):
            self.log("Events service not available, skipping...")
            return
            
        self.log("Testing Events Service...")
        
        try:
            events_service = self.camera.create_events_service()
            
            # Get Event Properties
            try:
                properties = events_service.GetEventProperties()
                self.record_test('GetEventProperties', True, "Retrieved event properties")
            except Exception as e:
                self.record_test('GetEventProperties', False, str(e))
                
        except Exception as e:
            self.record_test('EventsService', False, f"Failed to create events service: {str(e)}")
            
    def generate_report(self):
        """Generate final test report"""
        self.log("=" * 60)
        self.log("ONVIF Compliance Test Report")
        self.log("=" * 60)
        
        # Device Information
        self.log("\n📋 Device Information:")
        for key, value in self.results['device_info'].items():
            self.log(f"   {key.replace('_', ' ').title()}: {value}")
            
        # Available Services
        self.log("\n🔧 Available Services:")
        for service, available in self.results['services'].items():
            status = "✓" if available else "✗"
            self.log(f"   {status} {service.title()}")
            
        # Media Profiles
        if self.results['profiles']:
            self.log("\n📹 Media Profiles:")
            for profile in self.results['profiles']:
                self.log(f"   • {profile['name']}")
                if 'resolution' in profile:
                    self.log(f"     Resolution: {profile['resolution']}")
                if 'video_encoding' in profile:
                    self.log(f"     Encoding: {profile['video_encoding']}")
                if 'stream_uri' in profile:
                    self.log(f"     Stream URI: {profile['stream_uri']}")
                    
        # Test Summary
        self.log("\n📊 Test Summary:")
        total = self.results['summary']['passed'] + self.results['summary']['failed']
        self.log(f"   Passed: {self.results['summary']['passed']}/{total}", 'PASS')
        self.log(f"   Failed: {self.results['summary']['failed']}/{total}", 'FAIL')
        self.log(f"   Warnings: {self.results['summary']['warnings']}", 'WARN')
        
        # Save results to JSON
        report_file = f"onvif_report_{self.ip.replace('.', '_')}_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json"
        with open(report_file, 'w') as f:
            json.dump(self.results, f, indent=2, default=str)
        self.log(f"\n📄 Report saved to: {report_file}")
        
        return self.results
        
    def run_all_tests(self):
        """Run all ONVIF compliance tests"""
        self.log("=" * 60)
        self.log(f"ONVIF Compliance Tester")
        self.log(f"Target: {self.ip}:{self.port}")
        self.log("=" * 60)
        
        if not self.connect():
            self.log("Cannot proceed without camera connection", 'FAIL')
            return self.results
            
        self.test_device_service()
        self.test_media_service()
        self.test_ptz_service()
        self.test_imaging_service()
        self.test_events_service()
        
        return self.generate_report()


def main():
    """Main entry point"""
    if len(sys.argv) < 2:
        print("Usage: python onvif_tester.py <camera_ip> [port] [username] [password]")
        print("")
        print("Examples:")
        print("  python onvif_tester.py 192.168.1.100")
        print("  python onvif_tester.py 192.168.1.100 80")
        print("  python onvif_tester.py 192.168.1.100 80 admin password123")
        sys.exit(1)
        
    ip = sys.argv[1]
    port = int(sys.argv[2]) if len(sys.argv) > 2 else 80
    username = sys.argv[3] if len(sys.argv) > 3 else 'admin'
    password = sys.argv[4] if len(sys.argv) > 4 else 'admin'
    
    tester = ONVIFTester(ip, port, username, password)
    results = tester.run_all_tests()
    
    # Exit with appropriate code
    if results['summary']['failed'] > 0:
        sys.exit(1)
    sys.exit(0)


if __name__ == '__main__':
    main()
