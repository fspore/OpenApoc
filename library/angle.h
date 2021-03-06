
#pragma once

#include "vec.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace OpenApoc
{

template <typename T> class Angle
{
  private:
	T curAngle;

  public:
	Angle(T Degrees = 0);

	void Add(T Degrees);
	void Set(T Degrees);

	T ToDegrees();
	T ToRadians();

	T ShortestAngleTo(const Angle<T> DestinationAngle);
	T ShortestAngleTo(T DestinationAngle);
	bool ClockwiseShortestTo(const Angle<T> DestinationAngle);
	bool ClockwiseShortestTo(T DestinationAngle);
	void RotateShortestBy(const Angle<T> DestinationAngle, T ByDegrees);
	void RotateShortestBy(T DestinationAngle, T ByDegrees);

	T Sine();
	T Cosine();
	T Tan();
};

template <typename T> Angle<T>::Angle(T Degrees) : curAngle(Degrees) {}

template <typename T> void Angle<T>::Add(T Degrees)
{
	curAngle += Degrees;
	while (curAngle >= static_cast<T>(360))
	{
		curAngle -= static_cast<T>(360);
	}
	while (curAngle < static_cast<T>(0))
	{
		curAngle += static_cast<T>(360);
	}
}

template <typename T> void Angle<T>::Set(T Degrees) { curAngle = Degrees; }

template <typename T> T Angle<T>::ToDegrees() { return curAngle; }

template <typename T> T Angle<T>::ToRadians() { return glm::radians(curAngle); }

template <typename T> bool Angle<T>::ClockwiseShortestTo(const Angle<T> DestinationAngle)
{
	return ClockwiseShortestTo(DestinationAngle.ToDegrees());
}

template <typename T> bool Angle<T>::ClockwiseShortestTo(T DestinationAngle)
{
	T diff = DestinationAngle - curAngle;
	while (diff >= static_cast<T>(360))
	{
		diff -= static_cast<T>(360);
	}
	while (diff < static_cast<T>(0))
	{
		diff += static_cast<T>(360);
	}
	return (diff < static_cast<T>(180) && diff > static_cast<T>(0));
}

template <typename T> void Angle<T>::RotateShortestBy(const Angle<T> DestinationAngle, T ByDegrees)
{
	return RotateShortestBy(DestinationAngle.ToDegrees(), ByDegrees);
}

template <typename T> void Angle<T>::RotateShortestBy(T DestinationAngle, T ByDegrees)
{
	if (ClockwiseShortestTo(DestinationAngle))
	{
		Add(ByDegrees);
	}
	else
	{
		Add(-ByDegrees);
	}
}

template <typename T> T Angle<T>::ShortestAngleTo(const Angle<T> DestinationAngle)
{
	return ShortestAngleTo(DestinationAngle.ToDegrees());
}

template <typename T> T Angle<T>::ShortestAngleTo(T DestinationAngle)
{
	T ang = std::min(std::abs(DestinationAngle - curAngle), std::abs(curAngle - DestinationAngle));
	return ang;
}

template <typename T> T Angle<T>::Sine() { return sin(ToRadians()); }

template <typename T> T Angle<T>::Cosine() { return cos(ToRadians()); }

template <typename T> T Angle<T>::Tan() { return tan(ToRadians()); }

}; // namespace OpenApoc
