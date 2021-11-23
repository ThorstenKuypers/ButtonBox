/*
 * hal.h
 *
 * Created: 11/21/2021 11:55:20 PM
 *  Author: iceri
 */ 


#ifndef HAL_H_
#define HAL_H_

#include "commondefs.h"

namespace HAL {

template<byte_t reg>
class Register
{
	byte_t _reg;

	public:
	Register():_reg(reg){}

	void SetBit(byte_t bit)
	{
		_reg |= _BV(bit);
	}

	void ClearBit(byte_t bit)
	{
		_reg &= ~_BV(bit);
	}
};

}


#endif /* HAL_H_ */