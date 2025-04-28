#pragma once

#include "Collider.h"
#include "Body.h"
#include <array>
#include "Utility.h"

/**
 * @class ContactListener
 * @brief An abstract class for handling collision events.
 */
class ContactListener
{
public:
	/**
	 * @brief Called when a trigger begins.
	 * @param col1 The first collider ref involved in the collision.
	 * @param col2 The second collider ref involved in the collision.
	 */
	virtual void OnTriggerEnter(ColliderRef colRef1, ColliderRef colRef2) noexcept = 0;

	/**
	 * @brief Called when a trigger ends.
	 * @param col1 The first collider ref involved in the collision.
	 * @param col2 The second collider ref involved in the collision.
	 */
	virtual void OnTriggerExit(ColliderRef colRef1, ColliderRef colRef2) noexcept = 0;

	/**
	 * @brief Called when a collision begins.
	 * @param col1 The first collider involved in the collision.
	 * @param col2 The second collider involved in the collision.
	 */
	virtual void OnCollisionEnter(ColliderRef colRef1, ColliderRef colRef2) noexcept = 0;

	/**
	 * @brief Called when a collision ends.
	 * @param col1 The first collider involved in the collision.
	 * @param col2 The second collider involved in the collision.
	 */
	virtual void OnCollisionExit(ColliderRef colRef1, ColliderRef colRef2) noexcept = 0;
};

/**
 * @struct CollidingBody
 * @brief Represents a pair of colliding bodies.
 * @var CollidingBody::body The pointer to the body involved in the collision.
 * @var CollidingBody::collider The pointer to the collider associated with the body.
 */
struct CollidingBody
{
	Body* body = nullptr;
	Collider* collider = nullptr;
};

/**
 * @class Contact
 * @brief Represents a collision event and provides methods for resolving it.
 */

class Contact
{
public:
	std::array<CollidingBody, 2> CollidingBodies{}; // An array containing the two colliding bodies.

private:
	XMVECTOR Normal{}; // The collision normal vector.
	float Restitution{ 1 }; // The coefficient of restitution for the collision.
	float Penetration{}; // The penetration depth of the collision.

public:
	/**
	 * @brief Resolve the collision between two bodies.
	 */
	void Resolve();

private:
	/**
	 * @brief Calculate the separate velocity of the two colliding bodies.
	 * @return The separate velocity.
	 */
	float CalculateSeparateVelocity() const noexcept;

	/**
	 * @brief Resolve the velocity component of the collision.
	 */
	void ResolveVelocityAndInterpenetration()const noexcept;

	/**
	 * @brief Resolve the interpenetration between the two colliding bodies.
	 */
	void ResolveInterpenetration() const noexcept;

};