# NT98538 AI Samples Master Makefile
# Build all AI feature demo samples
#
# NOTE: These demos require vendor AI libraries (libpvdcnn, libfdcnn, etc.)
# which are not included in the open-source SDK. The samples are disabled
# by default to prevent build failures.
#
# To enable specific demos, ensure the required libraries are installed
# and uncomment the relevant sections below.

AI_SAMPLES_DIR := $(shell pwd)

# All targets are stubs - AI demos require vendor libraries not included
.PHONY: all clean install sample sample_install help

all:
	@echo "AI samples disabled - requires vendor libraries (libpvdcnn, libfdcnn, etc.)"

clean:
	@echo "AI samples clean - nothing to clean"

install:
	@echo "AI samples install - nothing to install"

sample:
	@echo "AI samples - disabled (requires vendor libraries)"

sample_install:
	@echo "AI samples install - disabled (requires vendor libraries)"

help:
	@echo "NT98538 AI Feature Demo Samples"
	@echo "================================"
	@echo ""
	@echo "NOTE: These demos are DISABLED because they require vendor AI libraries"
	@echo "      that are not included in the open-source SDK."
	@echo ""
	@echo "Required libraries (contact Novatek for availability):"
	@echo "  - libpvdcnn_lib  (Person/Vehicle Detection)"
	@echo "  - libfdcnn_lib   (Face Detection)"
	@echo "  - libmd          (Motion Detection)"
	@echo "  - libvqa_lib     (Video Quality Analysis)"
	@echo "  - libvendor_gfx  (Graphics/Mosaic)"
	@echo ""
	@echo "Available demos (when libraries are installed):"
	@echo "  ai_pvdcnn_demo           - Person/Vehicle Detection CNN"
	@echo "  ai_fdcnn_demo            - Face Detection CNN with landmarks"
	@echo "  ai_motion_detection_demo - MDBC, Global Alarm, Sub-Region"
	@echo "  ai_vqa_demo              - Video Quality Analysis"
	@echo "  ai_line_crossing_demo    - Line crossing detection"
	@echo "  ai_intrusion_detection_demo - Forbidden zone detection"
	@echo "  ai_privacy_mosaic_demo   - Real-time privacy masking"
	@echo "  ai_crowd_counting_demo   - Object/person counting"
	@echo "  ai_object_tracking_demo  - Multi-object tracking"
	@echo "  ai_cross_counting_demo   - Cross counting (in/out)"
	@echo "  ai_heat_map_demo         - Motion heat map generation"

