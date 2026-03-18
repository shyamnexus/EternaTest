#define TOKEN_LENGTH 64
#define VSMALL_BUFFER_LENGTH 10
#define BLC_FACTOR 10

typedef struct {
    int MAX_VALUE;
    int MIN_VALUE;
    int DEFAULT_VALUE;
} BrightnessConstants;

const BrightnessConstants BRIGHTNESS = {100, 1, 50};

// Define contrast constants
typedef struct {
    int MAX_VALUE;
    int MIN_VALUE;
    int DEFAULT_VALUE;
} ContrastConstants;

const ContrastConstants CONTRAST = {100, 1, 50};

// Define saturation constants
typedef struct {
    int MAX_VALUE;
    int MIN_VALUE;
    int DEFAULT_VALUE;
} SaturationConstants;

const SaturationConstants SATURATION = {100, 1, 50};

// Define sharpness constants
typedef struct {
    int MAX_VALUE;
    int MIN_VALUE;
    int DEFAULT_VALUE;
} SharpnessConstants;

const SharpnessConstants SHARPNESS = {100, 1, 50};

// Define back light compensation (BLC) constants
const int MIN_BLC_LEVEL = -50;
const int MAX_BLC_LEVEL = 50;

// Define wide dynamic range (WDR) constants
const int MIN_WDR_LEVEL = 1;
const int MAX_WDR_LEVEL = 7;

// Define red gain constants
const int MIN_RGAIN = 1;
const int MAX_RGAIN = 100;

// Define blue gain constants
const int MIN_BGAIN = 1;
const int MAX_BGAIN = 100;