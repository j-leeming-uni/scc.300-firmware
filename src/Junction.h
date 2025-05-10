#ifndef JUNCTION_H
#define JUNCTION_H

#include <Arduino.h>

#include "config.h"

namespace TrafficFlow {
    typedef enum { NONE, NORTH_SOUTH, EAST_WEST, NORTH, SOUTH, EAST, WEST } T;

    T next(T flow);

    T get_opposite(T flow);

    T with_opposite(T flow);

    void to_array(T flow, bool arr[4]);
} // namespace TrafficFlow

namespace Neighbour {
    typedef enum { NONE = -1, NORTH = 0, SOUTH, EAST, WEST } T;

    T next(T neighbour);

    TrafficFlow::T get_flow_for(T neighbour);
} // namespace Neighbour

struct JunctionState {
    uint8_t id : 4;
    uint8_t north : 1;
    uint8_t south : 1;
    uint8_t east : 1;
    uint8_t west : 1;

    static JunctionState from(uint8_t x);
    template<typename T>
    T to();

    IPAddress getIP() const;
};

class Junction {
private:
    JunctionState state = {0};
    JunctionState neighbours[4] = {0};

#if NON_REPUDIATION == 1
    uint8_t privateKey;
    uint8_t publicKey;
#endif

public:
    Junction(uint8_t id, uint8_t north, uint8_t south, uint8_t east, uint8_t west);

    IPAddress getIP() const;

    bool incoming(Neighbour::T neighbour) const;

    void setFlow(TrafficFlow::T flow);
    void update_neighbour(JunctionState state);
    void notify_neighbour(Neighbour::T neighbour);
    void notify_neighbours();

#if NON_REPUDIATION == 1
    char sign(char message) const;
    static bool verify(char message, char signature, uint8_t publicKey);
#endif

    void show();
};


#endif // JUNCTION_H
