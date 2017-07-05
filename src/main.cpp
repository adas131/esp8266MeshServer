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
#define LED D4

void receivedCallback(uint32_t from, String &msg);
void newConnectionCallback(size_t nodeId);
void dropConnectionCallback(size_t nodeId);
painlessMesh mesh;
SimpleList<uint32_t> nodes;

// Send my ID every 5 seconds to inform others
Task logServerTask(5000, TASK_FOREVER, []() {
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
    // mesh.setDebugMsgTypes(ERROR | CONNECTION | SYNC); // set before init() so that you can see startup messages

    mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT, STA_AP, AUTH_WPA2_PSK, 6);
    mesh.onReceive(&receivedCallback);
    mesh.onNewConnection(&newConnectionCallback);
    mesh.onDroppedConnection(&dropConnectionCallback);

    // Add the task to the mesh scheduler
    mesh.scheduler.addTask(logServerTask);
    logServerTask.enable();
}

void loop()
{
    mesh.update();
    SimpleList<uint32_t>::iterator node = nodes.begin();
    while (node != nodes.end())
    {
        mesh.startDelayMeas(*node);
        node++;
    }
    if (nodes.size() == 0)
        digitalWrite(LED, false);
    else
        digitalWrite(LED, true);
}

void newConnectionCallback(size_t nodeId)
{
    Serial.printf("New Connection %u\n", nodeId);
}

void dropConnectionCallback(size_t nodeId)
{
    Serial.printf("Drop Connection %u\n", nodeId);
}

void receivedCallback(uint32_t from, String &msg)
{
    Serial.printf("logServer: Received from %u msg=%s\n", from, msg.c_str());
}
