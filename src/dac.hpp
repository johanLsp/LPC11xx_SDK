#ifndef SRC_DAC_HPP_
#define SRC_DAC_HPP_

#include "core/LPC11xx.h"
#include "core/ssp.h"

namespace DAC {

enum DAC {DAC_A = 0, DAC_B};

void Init();
void Set(DAC dac, uint32_t value);
void Send(uint16_t* buffer, uint32_t size);

}  // namespace Dac

#endif  // SRC_DAC_HPP_
