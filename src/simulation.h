#ifndef SIMULATION_H
#define SIMULATION_H

#include "Junction.h"
#include "config.h"

#if SIMULATION == 1

inline int8_t getVehicleCount(const char* direction) {
    Serial.print("GET junction-");
    Serial.print(JUNCTION);
    Serial.print("::");
    Serial.print(direction);
    Serial.print("::vehicle_count\n");
    while (Serial.available() == 0) {}
    const char status = Serial.read();
    if (status == 'X') {
        return -1;
    }
    return Serial.parseInt();
}

inline TrafficFlow::T getNextFlowState(const TrafficFlow::T flow) {
    const int8_t northCount = getVehicleCount("north");
    const int8_t eastCount = getVehicleCount("east");
    const int8_t southCount = getVehicleCount("south");
    const int8_t westCount = getVehicleCount("west");

    if (northCount == -1 || eastCount == -1 || southCount == -1 || westCount == -1) {
        return next(flow);
    }

    const int north_south = northCount + southCount;
    const int east_west = eastCount + westCount;
    const int max_count = max(north_south, east_west);
    if (max_count == 0) {
        return TrafficFlow::NONE;
    }
    if (north_south > east_west) {
        return TrafficFlow::NORTH_SOUTH;
    }
    return TrafficFlow::EAST_WEST;
}

inline void setWorldState(const TrafficFlow::T flow) {
    bool buf[4];
    to_array(flow, buf);
    const char *directions[] = {"north", "east", "south", "west"};

    for (int i = 0; i < 4; i++) {
        Serial.print("SET junction-");
        Serial.print(JUNCTION);
        Serial.print("::");
        Serial.print(directions[i]);
        Serial.print("::light ");
        Serial.print(buf[i]);
        Serial.print("\n");
    }
}

#else

inline TrafficFlow::T getNextFlowState(const TrafficFlow::T flow) {
    return next(flow);
}

inline void setWorldState(TrafficFlow::T flow) {
    // Simulation is disabled, do nothing
}

#endif

#endif // SIMULATION_H
