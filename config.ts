/*********
 * Common
 */

/** Frequency of info (navdata, state and control) barter */
export var FREQUENCY = 50;

/** Domain (hostname or ip-address) */
export var DOMAIN = 'hdrone';

/** Port for video transferring */
export var VIDEO_PORT = 8081;

/** Port for transferring of state */
export var STATE_PORT = 8082;

/** Maximum timeout between activity for video */
export var VIDEO_TIMEOUT = 1000;

/** Maximum timeout between activity for state */
export var STATE_TIMEOUT = 1000;

/*************
 * Copter
 */

/** Periphery */
export var GY80_I2C_BUS = '/dev/i2c-0';
export var L3G4200D_RATE = 250;
export var ADXL345_RATE  = 50;
export var HMC5883L_RATE = 30;

/********
 * Video
 */

export var VIDEO_WIDTH  = 1024;
export var VIDEO_HEIGHT = 576;
export var VIDEO_FPS    = 24;

/** Bitrate of video stream. Set to 0 to disable control */
export var VIDEO_BITRATE = 2e6;

/** Size of GoP (group of pictures). Set to 0 to disable control */
export var VIDEO_GOP_SIZE = 0;

/** Whether to use horizontal flip */
export var VIDEO_HFLIP = true;

/** Whether to use vertical flip */
export var VIDEO_VFLIP = true;
