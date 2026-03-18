from onvif import ONVIFCamera
from datetime import datetime
import sys

def check_time(ip, user, password):
    try:
        cam = ONVIFCamera(ip, 80, user, password)
        dt = cam.devicemgmt.GetSystemDateAndTime()
        print(f" Camera Time: {dt.UTCDateTime.Date.Year}-{dt.UTCDateTime.Date.Month:02d}-{dt.UTCDateTime.Date.Day:02d} "
              f"{dt.UTCDateTime.Time.Hour:02d}:{dt.UTCDateTime.Time.Minute:02d}:{dt.UTCDateTime.Time.Second:02d}")
        print(f"System Time: {datetime.utcnow().strftime('%Y-%m-%d %H:%M:%S')}")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    check_time('192.168.0.3', 'admin', 'Admin@123')
