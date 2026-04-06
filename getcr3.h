#include <iostream>
#include <cstdint>
#include <cmath>
#include <chrono>
#include <thread>

uint64_t dtb = 0x0;
uint64_t Game_Base_Address = 0x0;


// set MAX_PHYADDR to a reasonable value, larger values will take more time to traverse.
constexpr uint64_t MAX_PHYADDR = 0xFFFFFFFFF;

// you should record and update it
uint64_t lastCorrectDtbPhysicalAddress = 0x0;

inline uint64_t GetFurtherDistance(uint64_t A, uint64_t Min, uint64_t Max) {
    uint64_t distanceToMin = (A > Min) ? (A - Min) : (Min - A);
    uint64_t distanceToMax = (A > Max) ? (A - Max) : (Max - A);

    return (distanceToMin > distanceToMax) ? distanceToMin : distanceToMax;
}

bool GetDTBSuccess(uint64_t dtbPhysicalAddress, uint64_t moduleBaseVirtualAddr)
{
    // This is dummy function, you should implement it to make it works
    // Through actual reading, return whether this dtbPhysicalAddress is the correct value.
    return 0x209dcf000 == dtbPhysicalAddress;
}
bool bruteforceDtb(uint64_t moduleBaseVirtualAddr, uint64_t dtbStartPhysicalAddr, const uint64_t stepPage, uint64_t* outDTBAddress)
{
    // eac cr3 always end with 0x-----XX000
    // dtbStartPhysicalAddr should be a multiple of 0x1000
    if ((dtbStartPhysicalAddr & 0xFFF) != 0) return false;
    if (dtbStartPhysicalAddr > MAX_PHYADDR) return false;

    dtbStartPhysicalAddr -= dtbStartPhysicalAddr % stepPage;
    dtbStartPhysicalAddr += lastCorrectDtbPhysicalAddress % stepPage;

    auto start = std::chrono::high_resolution_clock::now();
    bool result = false;
    uint64_t furtherDistance = GetFurtherDistance(dtbStartPhysicalAddr, 0x0, MAX_PHYADDR);
    size_t maxStep = furtherDistance / stepPage;
    // printf("[*] furtherDistance:0x%llx, maxStep:0x%zx\n", furtherDistance, maxStep);

    for (size_t step = 0; step < maxStep; step++) {
        // bruteforce dtb from middle
        uint64_t guessDtbAddr = dtbStartPhysicalAddr + step * stepPage;
        if (guessDtbAddr < MAX_PHYADDR) {
            if (GetDTBSuccess(guessDtbAddr, moduleBaseVirtualAddr)) {
                result = true;
                *outDTBAddress = guessDtbAddr;
                break;
            }
        }
        // dont forget the other side
        if (dtbStartPhysicalAddr > step * stepPage) {
            guessDtbAddr = dtbStartPhysicalAddr - step * stepPage;
            if (GetDTBSuccess(guessDtbAddr, moduleBaseVirtualAddr)) {
                result = true;
                *outDTBAddress = guessDtbAddr;
                break;
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    printf("[+] bruteforce dtb %s to find dtb:0x%llx, time use:%lldms\n", result ? "success" : "failed", result ? *outDTBAddress : 0x0, duration.count());

    // In case we cannot get the dtb through this shortcut method.
    if (result == false && stepPage != 0x1000) {
        return bruteforceDtb(moduleBaseVirtualAddr, dtbStartPhysicalAddr, 0x1000, outDTBAddress);
    }
    return result;
}