#pragma once

// Temporary: keeps code written against the old `Amanuensis::` namespace
// compiling while consumers move to `amanuensis::`. Every public header pulls
// this in, so consumers need no change until the sweep after Calamus lands,
// which also removes this file.
//
// A namespace alias can't be reopened, so `namespace Amanuensis { ... }` in a
// consumer (a forward declaration or a JsonTraits specialisation) still has to
// move to `namespace amanuensis`. Define AMANUENSIS_NO_COMPAT to check that a
// consumer no longer depends on the old name.

#ifndef AMANUENSIS_NO_COMPAT
namespace amanuensis {}
namespace Amanuensis = amanuensis; // NOLINT(misc-unused-alias-decls): used by consumers
#endif
