#pragma once
#include "Shapes.h"
#include "Physics.h"

#include <cstdint>
#include <glm/gtc/quaternion.hpp>

namespace Glory
{
	class CharacterManager
	{
	public:
		uint32_t CreateCharacter(float maxSlopeAngle, uint32_t layerIndex, const glm::vec3& inPosition, const glm::quat& inRotation, const Shape& shape, float friction);
		void DestroyCharacter(uint32_t characterID);

		void DestroyAll();

		/// Wake up the character
		virtual void Activate(uint32_t characterID, bool lockBodies = true) = 0;

		/// Control the velocity of the character
		virtual void SetLinearAndAngularVelocity(uint32_t characterID, const glm::vec3& linearVelocity, const glm::vec3& angularVelocity, bool lockBodies = true) = 0;

		/// Get the linear velocity of the character (m / s)
		virtual glm::vec3 GetLinearVelocity(uint32_t characterID, bool lockBodies = true) const = 0;

		/// Set the linear velocity of the character (m / s)
		virtual void SetLinearVelocity(uint32_t characterID, const glm::vec3& linearVelocity, bool lockBodies = true) = 0;

		/// Add world space linear velocity to current velocity (m / s)
		virtual void AddLinearVelocity(uint32_t characterID, const glm::vec3& linearVelocity, bool lockBodies = true) = 0;

		/// Add impulse to the center of mass of the character
		virtual void AddImpulse(uint32_t characterID, const glm::vec3& impulse, bool lockBodies = true) = 0;

		/// Get the body associated with this character
		virtual uint32_t GetBodyID(uint32_t characterID) const = 0;

		/// Get position / rotation of the body
		virtual void GetPositionAndRotation(uint32_t characterID, glm::vec3& outPosition, glm::quat& outRotation, bool lockBodies = true) const = 0;

		/// Set the position / rotation of the body, optionally activating it.
		virtual void SetPositionAndRotation(uint32_t characterID, const glm::vec3& position, const glm::quat& rotation, ActivationType activationMode = ActivationType::Activate, bool lockBodies = true) const = 0;

		/// Get the position of the character
		virtual glm::vec3 GetPosition(uint32_t characterID, bool lockBodies = true) const = 0;

		/// Set the position of the character, optionally activating it.
		virtual void SetPosition(uint32_t characterID, const glm::vec3& position, ActivationType activationMode = ActivationType::Activate, bool lockBodies = true) = 0;

		/// Get the rotation of the character
		virtual glm::quat GetRotation(uint32_t characterID, bool lockBodies = true) const = 0;

		/// Set the rotation of the character, optionally activating it.
		virtual void SetRotation(uint32_t characterID, const glm::quat& rotation, ActivationType activationMode = ActivationType::Activate, bool lockBodies = true) = 0;

		/// Position of the center of mass of the underlying rigid body
		virtual glm::vec3 GetCenterOfMassPosition(uint32_t characterID, bool lockBodies = true) const = 0;

		/// Update the layer of the character
		virtual void SetLayer(uint32_t characterID, uint32_t layer, bool lockBodies = true) = 0;

		/// Switch the shape of the character (e.g. for stance). When inMaxPenetrationDepth is not FLT_MAX, it checks
		/// if the new shape collides before switching shape. Returns true if the switch succeeded.
		virtual bool SetShape(uint32_t characterID, const Shape& shape, float maxPenetrationDepth, bool lockBodies = true) = 0;

		/// @brief Get all contacts for the character at a particular location
		/// @param inPosition Position to test.
		/// @param inRotation Rotation at which to test the shape.
		/// @param inMovementDirection A hint in which direction the character is moving, will be used to calculate a proper normal.
		/// @param inMaxSeparationDistance How much distance around the character you want to report contacts in (can be 0 to match the character exactly).
		/// @param inShape Shape to test collision with.
		/// @param inBaseOffset All hit results will be returned relative to this offset, can be zero to get results in world position, but when you're testing far from the origin you get better precision by picking a position that's closer e.g. GetPosition() since floats are most accurate near the origin
		/// @param ioCollector Collision collector that receives the collision results.
		/// @param inLockBodies If the collision query should use the locking body interface (true) or the non locking body interface (false)
		//virtual void CheckCollision(uint32_t characterID, const glm::vec3& inPosition, QuatArg inRotation, const glm::vec3& inMovementDirection, float inMaxSeparationDistance, const Shape* inShape, const glm::vec3& inBaseOffset, CollideShapeCollector& ioCollector, bool inLockBodies = true) const = 0;

	protected:
		virtual uint32_t CreateCharacter_Internal(float maxSlopeAngle, uint32_t layerIndex, const glm::vec3& position, const glm::quat& rotation, const Shape& shape, float friction) = 0;
		virtual void DestroyCharacter_Internal(uint32_t) = 0;
		virtual void DestroyAll_Internal() = 0;
	};
}
