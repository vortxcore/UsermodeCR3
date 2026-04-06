#include "getcr3.h"
 
int main() {

    // do not set stepPage more than 0x100000
    while (true) {
        if (bruteforceDtb(Game_Base_Address, 0x300000000, 0x100000, &dtb)) {
            lastCorrectDtbPhysicalAddress = dtb;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}