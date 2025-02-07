// Copyright (C) 2016 Y_Less
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#if defined ADDRESSOF_INC
	#endinput
#endif
#define ADDRESSOF_INC

/**
 * <library name="amx_assembly addressof" summary="AMX Assembly Library: `addressof` function address lookup.">
 *   <summary pawndoc="true">
 *     This library uses the enhanced <em>pawndoc.xsl</em> from
 *     <a href="https://github.com/pawn-lang/pawndoc">pawn-lang/pawndoc</a>.
 *     This XSL has features such as library and markdown support, and will not
 *     render this message when used.
 *   </summary>
 * </library>
 */

/// <p/>

#include "frame_info"
#include "disasm"

// This code uses two nested conditionals because of:
// https://github.com/Zeex/pawn/issues/96 (this doesn't work):
//   
//   (O@A_() ? ((CALL@%1), 0) : (O@V_))
//   
// Even though it is the obvious solution when you don't want the result of
// "CALL@%1" to be used (as it may not exist), and you can't use a constant
// instead of "O@V_" because then if becomes a constant in a
// condition, which the compiler rightly complains about.  Of course, because
// "O@A_()" always returns "false", the entire other block of code
// is jumped over.
#define CALL@%0\32; CALL@
#define gAddressOfReturnVar_ O@V_

#define addressof(%1) (O@A_()?(((CALL@%1),O@V_)?1:2):(O@V_))

#define AddressOfGetNextCall_ O@A_
#define CALL@AddressOfGetNextCall_ CALL@O@A_
#define CALL@O@A_ O@A_()

#define O@A_()?(((CALL@%1<%2>),O@V_)?1:2):(O@V_) (O@A_()?(((_ADDR@$%1()<%2>),O@V_)?(F@_@:1):(F@_@:2)):(F@_@:O@V_))

#define nativeidxof(%1) (_:(O@B_()?(((CALL@%1),O@V_)?1:2):(O@V_)))

#define NativeIdxOfGetNextCall_ O@B_
#define CALL@NativeIdxOfGetNextCall_ CALL@O@B_
#define CALL@O@B_ O@B_()

#define O@B_()?(((CALL@%1<%2>),O@V_)?1:2):(O@V_) (O@B_()?(((_ADDR@$%1()<%2>),O@V_)?(F@_@:1):(F@_@:2)):(F@_@:O@V_))

// Enable parameter checks for `Func<xxx>` types.
#define _ADDR@ _ADDR@i:_ADDR@t:_ADDR@f:_ADDR@v:_ADDR@s:_ADDR@a:_ADDR@x:_ADDR@z

#define _ADDR@z$%0(%1)<%2> _ADDR@y:%0(%1)
#define _ADDR@y:%0(,%1) %0(%1)

#define _ADDR@i:%8$%0(%1)<i%2>),O@V_)?(%9:1):(%9:2)):(%9:O@V_)) _ADDR@$%0(%1,0)<%2>),O@V_)?(%9i:1):(%9i:2)):(%9i:O@V_))
#define _ADDR@t:%8$%0(%1)<t%5:%2>),O@V_)?(%9:1):(%9:2)):(%9:O@V_)) _ADDR@$%0(%1,%5:0)<%2>),O@V_)?(%9i:1):(%9i:2)):(%9i:O@V_))
#define _ADDR@f:%8$%0(%1)<f%2>),O@V_)?(%9:1):(%9:2)):(%9:O@V_)) _ADDR@$%0(%1,0.0)<%2>),O@V_)?(%9f:1):(%9f:2)):(%9f:O@V_))
#define _ADDR@v:%8$%0(%1)<v%2>),O@V_)?(%9:1):(%9:2)):(%9:O@V_)) _ADDR@$%0(%1,__REF)<%2>),O@V_)?(%9v:1):(%9v:2)):(%9v:O@V_))
#define _ADDR@s:%8$%0(%1)<s%2>),O@V_)?(%9:1):(%9:2)):(%9:O@V_)) _ADDR@$%0(%1,__ARR)<%2>),O@V_)?(%9s:1):(%9s:2)):(%9s:O@V_))
#define _ADDR@a:%8$%0(%1)<a%2>),O@V_)?(%9:1):(%9:2)):(%9:O@V_)) _ADDR@$%0(%1,__ARR)<%2>),O@V_)?(%9a:1):(%9a:2)):(%9a:O@V_))
#define _ADDR@x:%8$%0(%1)<x%2>),O@V_)?(%9:1):(%9:2)):(%9:O@V_)) _ADDR@z$%0(%1)<>),O@V_)?(%9x:1):(%9x:2)):(%9x:O@V_))

/// <library>amx_assembly addressof</library>
stock __ARR[1] = {};

/// <library>amx_assembly addressof</library>
stock __REF = 0;

/// <library>amx_assembly addressof</library>
stock gAddressOfReturnVar_ = 0;

/// <library>amx_assembly addressof</library>
stock bool:AddressOfGetNextCall_() {
	// Start reading code from the point to which this function returns, looking
	// for the next "CALL" op to signal the function call from the macro.
	new ctx[DisasmContext];
	DisasmInit(ctx, GetCurrentFrameReturn());
	while (DisasmNext(ctx)) {
		if (ctx[DisasmContext_opcode] == OP_CALL) {
			// Return the data in a global, to be repassed from the conditional.
			gAddressOfReturnVar_ = DisasmGetOperandReloc(ctx);
			return false;
		}
	}
	// ALWAYS returns false so that the function call within "OP(&func)" will
	// never be called thanks to the conditional.
	return false;
}

/// <library>amx_assembly v</library>
stock bool:NativeIdxOfGetNextCall_() {
	// Start reading code from the point to which this function returns, looking
	// for the next "CALL" op to signal the function call from the macro.
	new ctx[DisasmContext];
	DisasmInit(ctx, GetCurrentFrameReturn());
	while (DisasmNext(ctx)) {
		if (ctx[DisasmContext_opcode] == OP_SYSREQ_C) {
			// Return the data in a global, to be repassed from the conditional.
			gAddressOfReturnVar_ = DisasmGetOperand(ctx);
			return false;
		}
	}
	// ALWAYS returns false so that the function call within "OP(&func)" will
	// never be called thanks to the conditional.
	return false;
}

