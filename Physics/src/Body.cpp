#include "Body.h"

void Body::ApplyForce(const XMVECTOR &force) noexcept
{
    _force = XMVectorAdd(_force, force);
}