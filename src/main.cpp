#include <Arduino.h>
#include <Ethernet.h>

#include "Junction.h"
#include "config.h"
#include "simulation.h"

#if JUNCTION == 1
Junction junction(1, 0, 0, 2, 0);
#elif JUNCTION == 2
Junction junction(2, 0, 0, 0, 1);
#endif

#define PIN_WRITE(PIN, VALUE)                                                                                          \
    do {                                                                                                               \
        digitalWrite(PIN, VALUE);                                                                                      \
        digitalWrite(LED_##PIN, VALUE);                                                                                \
    } while (0)


EthernetServer server(SMART_CONNECTION_PORT);

TrafficFlow::T flowDirection = TrafficFlow::NORTH;

uint32_t lastUpdateTime = 0, lastSmartUpdateTime = 0;
bool hasChanged = false;

void setup() {
    pinMode(LED_D0, OUTPUT);
    pinMode(LED_D1, OUTPUT);
    pinMode(LED_D2, OUTPUT);
    pinMode(LED_D3, OUTPUT);

    pinMode(D0, OUTPUT);
    pinMode(D1, OUTPUT);
    pinMode(D2, OUTPUT);
    pinMode(D3, OUTPUT);

    Serial.begin(9600);

    pinMode(LEDR, OUTPUT);
    pinMode(LED_USER, OUTPUT);

    digitalWrite(LED_USER, HIGH);

    pinMode(BTN_USER, INPUT);
    bool ledState = true;
    while (digitalRead(BTN_USER) == HIGH) {
        digitalWrite(LEDR, ledState);
        ledState = !ledState;
        delay(100);
    }
    digitalWrite(LEDR, LOW);

    delay(250);
    digitalWrite(LED_USER, LOW);

    Ethernet.begin(junction.getIP());
    server.begin();
}

void check_peers() {
    EthernetClient client = server.available();
    if (!client)
        return;

    while (client.connected()) {
        const char c = client.read();
        if (c == static_cast<char>(-1))
            continue;
        const JunctionState state = JunctionState::from(c);

#if NON_REPUDIATION == 1
        const char publicKey = client.read();
        if (publicKey == static_cast<char>(-1))
            continue;
        const char signature = client.read();
        if (signature == static_cast<char>(-1))
            continue;

        if (!Junction::verify(c, signature, publicKey)) {
            digitalWrite(LEDR, HIGH);
            delay(250);
            digitalWrite(LEDR, LOW);
            break;
        }
#endif

        junction.update_neighbour(state);
        break;
    }
    client.stop();
}

void update() {
    const uint32_t now = millis();

    static Neighbour::T ignore = Neighbour::NONE;

    // Notify neighbours if the flow has changed
    if (hasChanged) {
        junction.setFlow(flowDirection);
        setWorldState(flowDirection);

        if (ignore != Neighbour::NORTH)
            junction.notify_neighbour(Neighbour::NORTH);
        if (ignore != Neighbour::SOUTH)
            junction.notify_neighbour(Neighbour::SOUTH);
        if (ignore != Neighbour::EAST)
            junction.notify_neighbour(Neighbour::EAST);
        if (ignore != Neighbour::WEST)
            junction.notify_neighbour(Neighbour::WEST);

        hasChanged = false;
        ignore = Neighbour::NONE;
    }

    // Spend at most 2 seconds between updates
    if (now - lastUpdateTime > 2000) {
        lastUpdateTime = now;
        flowDirection = getNextFlowState(flowDirection);
        hasChanged = true;
        return;
    }

    // Try smart update once per second
    if (now - lastSmartUpdateTime < 1000)
        return;
    lastSmartUpdateTime = now;

    static Neighbour::T start = Neighbour::NORTH;

    // Check each neighbour. If that neighbour is incoming, switch to let them through
    int i = 0;
    for (Neighbour::T n = start; i < 4; n = next(n), i++) {
        // Only update the lastUpdateTime if we actually switch the flow
        if (junction.incoming(n)) {
            const TrafficFlow::T newFlow = get_opposite(get_flow_for(n));
            if (flowDirection != newFlow) {
                flowDirection = newFlow;
                lastUpdateTime = now;
                hasChanged = true;
                ignore = n;
            }
            break;
        }
    }

    start = next(start);
}

void render() {
    switch (flowDirection) {
        case TrafficFlow::NONE:
            PIN_WRITE(D0, LOW);
            PIN_WRITE(D1, LOW);
            PIN_WRITE(D2, LOW);
            PIN_WRITE(D3, LOW);
            break;
        case TrafficFlow::NORTH_SOUTH:
            PIN_WRITE(D0, HIGH);
            PIN_WRITE(D1, LOW);
            PIN_WRITE(D2, HIGH);
            PIN_WRITE(D3, LOW);
            break;
        case TrafficFlow::EAST_WEST:
            PIN_WRITE(D0, LOW);
            PIN_WRITE(D1, HIGH);
            PIN_WRITE(D2, LOW);
            PIN_WRITE(D3, HIGH);
            break;
        case TrafficFlow::NORTH:
            PIN_WRITE(D0, HIGH);
            PIN_WRITE(D1, LOW);
            PIN_WRITE(D2, LOW);
            PIN_WRITE(D3, LOW);
            break;
        case TrafficFlow::SOUTH:
            PIN_WRITE(D0, LOW);
            PIN_WRITE(D1, LOW);
            PIN_WRITE(D2, HIGH);
            PIN_WRITE(D3, LOW);
            break;
        case TrafficFlow::EAST:
            PIN_WRITE(D0, LOW);
            PIN_WRITE(D1, HIGH);
            PIN_WRITE(D2, LOW);
            PIN_WRITE(D3, LOW);
            break;
        case TrafficFlow::WEST:
            PIN_WRITE(D0, LOW);
            PIN_WRITE(D1, LOW);
            PIN_WRITE(D2, LOW);
            PIN_WRITE(D3, HIGH);
            break;
    }
}


void loop() {
    check_peers();
    update();
    render();
}
