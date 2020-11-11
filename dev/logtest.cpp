#include "common.h"
#include "logging.h"

int main() {
    int logsize = 1024;
    Log<int> logi = Log<int>(NULL);

    int datapoints[] = {13, 14, 15, 2000, 3333};
    time_t timestamps[] = {1603723663, 1603723700, 1603724000, 1603724000, 1603724100};

    for (int i = 0; i < 5; i++) {
        logi.log(datapoints + i, timestamps[i]);
    }

    Success_Handler();
}
