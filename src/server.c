/*
 * Copyright (c) 2020 Riptide IO, Inc.
 * All rights reserved.
 */
#include <string.h>

#include "comm.h"
#include "debug.h"

#include "server.h"

static unsigned char buffer[1024];
static unsigned char *packets[] = {
        "Don't Panic.",
        "Time is an illusion. Lunchtime doubly so.",
        "Would it save you a lot of time if I just gave up and went mad now?",
        "Isn't it enough to see that a garden is beautiful without having to believe that there are fairies at the bottom of it too?",
        "The ships hung in the sky in much the same way that bricks don't.",
        "He felt that his whole life was some kind of dream and he sometimes wondered whose it was and whether they were enjoying it.",
        "I'd far rather be happy than right any day.",
        "Space is big. You just won't believe how vastly, hugely, mind-bogglingly big it is. I mean, you may think it's a long way down the road to the chemist's, but that's just peanuts to space.",
        "If there's anything more important than my ego around, I want it caught and shot now.",
        "For a moment, nothing happened. Then, after a second or so, nothing continued to happen.",
        "This must be Thursday,' said Arthur to himself, sinking low over his beer. 'I never could get the hang of Thursdays.",
        "The Answer to the Great Question... Of Life, the Universe and Everything... Is... Forty-two,' said Deep Thought, with infinite majesty and calm.",
        "So long, and thanks for all the fish.",
        "Anyone who is capable of getting themselves made President should on no account be allowed to do the job."
};
static size_t n_packets = sizeof(packets)/sizeof(unsigned char *);

void server_loop(configuration *config, interface_struct *interface) {
    debug_printf("server_loop\n");
    while (1) {
        for (int i=0; i<n_packets; i++) {
            unsigned char *packet = packets[i];
            size_t packet_length = strlen(packet) + 1;
            send_packet(interface, packet, packet_length);
            if (recv_packet(interface, buffer, sizeof(buffer))) {
                if (strcmp(packet, buffer) == 0) {
                    debug_printf("valid echo.\n");
                    continue;
                }
                debug_printf("invalid echo.");
                continue;
            }
        }
    }
}
