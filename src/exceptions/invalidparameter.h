#ifndef INVALIDPARAMETER_H
#define INVALIDPARAMETER_H
#include "logic.h"
#include "config.h"

namespace Exceptions
{
    /** @class InvalidParameter invalidparameter.h
     * @author 0xd34df00d
     * @ingroup MEx
     * @brief Invalid Parameter exception.
     * Should be thrown when a method detects that client has passed
     * an invalid parameter to it, and it doesn't belong to
     * OutOfBounds kind of errors.
     *
     * @sa Logic, OutOfBounds
     */
    class LEECHCRAFT_API InvalidParameter : public Logic
    {
    public:
		LEECHCRAFT_API InvalidParameter (const std::string& reason = std::string ()) throw ();
		LEECHCRAFT_API virtual ~InvalidParameter () throw ();
    };
};

#endif

