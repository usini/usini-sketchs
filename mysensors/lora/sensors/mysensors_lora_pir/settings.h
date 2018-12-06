#define MY_ENCRYPTION_SIMPLE_PASSWD "MyInsecurePassword"
#define MY_NODE_ID 4 // Static ID
#define MY_PARENT_NODE_ID 1 // Static ID
#define CHILD_PIR 1 

// MiniPro
#define MY_RADIO_RFM95
#define MY_RFM95_ENABLE_ENCRYPTION

//Lora32u4
#define MY_RADIO_RFM95
#define MY_RFM95_ENABLE_ENCRYPTION
#define MY_RFM95_IRQ_PIN (7)
#define MY_RFM95_IRQ_NUM digitalPinToInterrupt(7) 
#define MY_RFM95_CS_PIN 8
