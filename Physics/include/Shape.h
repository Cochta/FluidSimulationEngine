#pragma once

#include <DirectXMath.h>
#include "Utility.h"
#include <vector>

using namespace DirectX;

enum class ShapeType
{
	Sphere, Cuboid, None
};

template <typename T>
class Sphere
{
public:
	/**
	 * @brief Construct a new Sphere object
	 * @param center Center of the sphere, also the position of the sphere
	 * @param radius Radius of the sphere, if negative, it will be converted to its positive value
	 */
	constexpr Sphere(XMVECTOR center, T radius) noexcept : _center(center), _radius(Abs(radius)) {}

private:
	XMVECTOR _center = XMVectorZero();
	T _radius = 0;

public:
	[[nodiscard]] constexpr XMVECTOR Center() const noexcept { return _center; }
	[[nodiscard]] constexpr T Radius() const noexcept { return _radius; }

	void SetCenter(XMVECTOR center) noexcept { _center = center; }
	void SetRadius(T radius) noexcept { _radius = Abs(radius); }

	/**
	 * @brief Check if the sphere contains a point
	 * @param point the point to check
	 * @return true if the point is inside the sphere, false otherwise
	 */
	 /*[[nodiscard]] bool Contains(XMVECTOR point) const
	 {
		 return (_center - point).SquareLength() <= _radius * _radius;
	 }*/

	[[nodiscard]] bool Contains(XMVECTOR point) const {
		// Compute the vector difference between the sphere's center and the point
		XMVECTOR delta = XMVectorSubtract(_center, point);

		// Calculate the squared distance (squared length of delta)
		float squaredDistance = XMVectorGetX(XMVector3LengthSq(delta));

		// Compare squared distance with squared radius
		return squaredDistance <= (_radius * _radius);
	}

	[[nodiscard]] constexpr Sphere<T> operator+(const XMVECTOR& vec) const noexcept
	{
		return Sphere<T>(XMVectorAdd(_center, vec), _radius);
	}
};

using SphereF = Sphere<float>;
using sphereI = Sphere<int>;

template <typename T>
class Cuboid
{
public:
	/**
	 * @brief Construct a new Cuboid object
	 * @param position the position of the cuboid
	 * @param size the size of the cuboid
	 */
	constexpr Cuboid(XMVECTOR minBound, XMVECTOR maxBound) noexcept : _minBound(minBound), _maxBound(maxBound) {}

private:
	XMVECTOR _minBound = cheval;
	XMVECTOR _maxBound = XMVectorZero();

public:
	[[nodiscard]] constexpr XMVECTOR MinBound() const noexcept { return _minBound; }
	[[nodiscard]] constexpr XMVECTOR MaxBound() const noexcept { return _maxBound; }

	void SetMinBound(XMVECTOR minBound) noexcept { _minBound = minBound; }
	void SetMaxBound(XMVECTOR maxBound) noexcept { _maxBound = maxBound; }

	/**
	 * @brief Check if the cuboid contains a point
	 * @param point the point to check
	 * @return true if the point is inside the cuboid, false otherwise
	 */
	[[nodiscard]] bool Contains(XMVECTOR point) const
	{
		if (XMVectorGetX(_maxBound) < XMVectorGetX(point) || XMVectorGetX(_minBound) > XMVectorGetX(point)) return false;
		if (XMVectorGetY(_maxBound) < XMVectorGetY(point) || XMVectorGetY(_minBound) > XMVectorGetY(point)) return false;
		if (XMVectorGetZ(_maxBound) < XMVectorGetZ(point) || XMVectorGetZ(_minBound) > XMVectorGetZ(point)) return false;

		return true;
	}

	[[nodiscard]] constexpr XMVECTOR Center() const noexcept
	{
		return XMVectorScale(XMVectorAdd(_minBound, _maxBound), 0.5f);
	}

	[[nodiscard]] constexpr XMVECTOR Size() const noexcept
	{
		return XMVectorSubtract(_maxBound, _minBound);
	}

	[[nodiscard]] constexpr XMVECTOR HalfSize() const noexcept
	{
		return XMVectorScale(XMVectorSubtract(_maxBound, _minBound), 0.5f);
	}

	[[nodiscard]] constexpr Cuboid<T> operator+(const XMVECTOR& vec) const noexcept
	{
		return Cuboid<T>(XMVectorAdd(_minBound, vec), XMVectorAdd(_maxBound, vec));
	}

	[[nodiscard]] static constexpr Cuboid<T> FromCenter(XMVECTOR center, XMVECTOR halfSize) noexcept
	{
		return Cuboid<T>(XMVectorSubtract(center, halfSize), XMVectorAdd(center, halfSize));
	}
};

using CuboidF = Cuboid<float>;
using CuboidI = Cuboid<int>;

// Intersect functions

template<typename T>
[[nodiscard]] constexpr bool Intersect(const Sphere<T> sphere1, const Sphere<T> sphere2) noexcept
{
	const T radiusSum = (sphere1.Radius() + sphere2.Radius()) * (sphere1.Radius() + sphere2.Radius());

	const T distanceBetweenCenters = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(sphere1.Center(), sphere2.Center())));
	return distanceBetweenCenters <= radiusSum;
}

template<typename T>
[[nodiscard]] constexpr bool Intersect(const Cuboid<T> cuboid1, const Cuboid<T> cuboid2) noexcept
{
	if (XMVectorGetX(cuboid1.MaxBound()) < XMVectorGetX(cuboid2.MinBound()) ||
		XMVectorGetX(cuboid1.MinBound()) > XMVectorGetX(cuboid2.MaxBound()))
		return false;
	if (XMVectorGetY(cuboid1.MaxBound()) < XMVectorGetY(cuboid2.MinBound()) ||
		XMVectorGetY(cuboid1.MinBound()) > XMVectorGetY(cuboid2.MaxBound()))
		return false;
	if (XMVectorGetZ(cuboid1.MaxBound()) < XMVectorGetZ(cuboid2.MinBound()) ||
		XMVectorGetZ(cuboid1.MinBound()) > XMVectorGetZ(cuboid2.MaxBound()))
		return false;

	return true;
}

template <typename T>
[[nodiscard]] constexpr bool Intersect(const Cuboid<T>& cuboid, const Sphere<T>& sphere) noexcept {
	XMVECTOR closestPoint = XMVectorClamp(
		sphere.Center(),
		cuboid.MinBound(),
		cuboid.MaxBound()
	);

	float distanceSq = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(closestPoint, sphere.Center())));
	return distanceSq <= (sphere.Radius() * sphere.Radius());
}

template <typename T>
[[nodiscard]] constexpr bool Intersect(const Sphere<T> sphere, const Cuboid<T> cuboid) noexcept
{
	return Intersect(cuboid, sphere);
}