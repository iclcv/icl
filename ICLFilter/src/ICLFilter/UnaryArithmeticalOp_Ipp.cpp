// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLCore/ImageBackendDispatching.h>
#include <ICLCore/Image.h>
#include <ICLCore/Visitors.h>
#include <ICLUtils/ClippedCast.h>
#include <ICLFilter/UnaryArithmeticalOp.h>

// IPP backends to be added when building on a platform with IPP
// Registration pattern:
//   using UAOp = icl::filter::UnaryArithmeticalOp;
//   using Op = UAOp::Op;
//   auto ipp = UAOp::prototype().backends(Backend::Ipp);
//   ipp.add<UAOp::ArithValSig>(Op::withVal, ipp_fn, applicableTo<...>, "...");
//   ipp.add<UAOp::ArithNoValSig>(Op::noVal, ipp_fn, applicableTo<...>, "...");
