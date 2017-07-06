//************************************************************
// this is a simple example that uses the painlessMesh library to
// setup a single node (this node) as a logging node
// The logClient example shows how to configure the other nodes
// to log to this server
//************************************************************
#include <painlessMesh.h>

#define MESH_PREFIX "whateverYouLike"
#define MESH_PASSWORD "somethingSneaky"
#define MESH_PORT 5555

void receivedCallback(uint32_t from, String &msg);
void serialLoop();

painlessMesh mesh;
String serialString;

// Send my ID every 10 seconds to inform others
Task logServerTask(10000, TASK_FOREVER, []() {
    DynamicJsonBuffer jsonBuffer;
    JsonObject &msg = jsonBuffer.createObject();
    msg["topic"] = "logServer";
    msg["nodeId"] = mesh.getNodeId();

    String str;
    msg.printTo(str);
    mesh.sendBroadcast(str);

    // log to serial
    msg.printTo(Serial);
    Serial.printf("\n");
});

void setup()
{
    Serial.begin(115200);

    //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE | DEBUG ); // all types on
    //mesh.setDebugMsgTypes( ERROR | CONNECTION | SYNC | S_TIME );  // set before init() so that you can see startup messages
    mesh.setDebugMsgTypes(ERROR | CONNECTION | S_TIME); // set before init() so that you can see startup messages

    mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT, STA_AP, AUTH_OPEN, 2);
    mesh.onReceive(&receivedCallback);

    mesh.onNewConnection([](size_t nodeId) {
        Serial.printf("New Connection %u\n", nodeId);
    });

    mesh.onDroppedConnection([](size_t nodeId) {
        Serial.printf("Dropped Connection %u\n", nodeId);
    });

    // Add the task to the mesh scheduler
    mesh.scheduler.addTask(logServerTask);
    logServerTask.enable();
}

void loop()
{
    mesh.update();
    serialLoop();
}

void receivedCallback(uint32_t from, String &msg)
{
    Serial.printf("%u logServer: Received from %u msg=%s\n", millis(), from, msg.c_str());
}

void serialLoop()
{
    while (Serial.available())
    {
        char inChar = Serial.read();
        serialString += inChar;

        if (inChar == '\n')
        { // truncate and parse Json
            mesh.sendBroadcast(serialString);
            serialString = "";
            break;
        }
    }
}