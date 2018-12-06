const String usb_name = "usini_radio433_gateway";
const float VER = 1.3;
const int BAUDRATE = 9600;

//Radio
const int txPin = 10; //Transmitter (You can change where it is plugged)
const int rxPin = 2; //Receiver (We need to use Interrupt 0 ==> Pin 2)

const int REPEATS = 2; //Repeats for /radio/new and /radio/old
const int BITRATE_RADIOHEAD = 2000; //RadioHead bitrate
