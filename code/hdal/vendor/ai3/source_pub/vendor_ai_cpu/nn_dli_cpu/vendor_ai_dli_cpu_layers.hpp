/**
	@brief Header file of definition of all DLI cpu layers.

	@file vendor_ai_dli_cpu_layers.hpp

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _VENDOR_AI_DLI_CPU_LAYERS_HPP_
#define _VENDOR_AI_DLI_CPU_LAYERS_HPP_

#if NN_DLI_CPU_ACL
#include "vendor_ai_dli_cpu_elementwise.hpp"
#include "vendor_ai_dli_cpu_resize.hpp"
#endif
#include "vendor_ai_dli_cpu_softmax.hpp"
#include "vendor_ai_dli_cpu_pooling.hpp"
#include "vendor_ai_dli_cpu_reverse.hpp"
#include "vendor_ai_dli_cpu_permute.hpp"


#endif /* _VENDOR_AI_DLI_CPU_LAYERS_HPP_ */
