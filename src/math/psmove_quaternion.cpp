/**
* PS Move API - An interface for the PS Move Motion Controller
* Copyright (c) 2011, 2012 Thomas Perl <m@thp.io>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*    1. Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*
*    2. Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
**/

//-- includes ----
#include "psmove_quaternion.hpp"

#include <assert.h>

//-- globals ----
const Eigen::Quaternionf g_psmove_quaternion_zero = Eigen::Quaternionf( 0, 0, 0, 0 );
const Eigen::Quaternionf *k_psmove_quaternion_zero = &g_psmove_quaternion_zero;

//-- public methods ----
Eigen::Quaternionf
psmove_quaterion_yaw_pitch_roll(float yaw_radians, float pitch_radians, float roll_radians)
{
	// Assuming the angles are in radians.
	//(x=pitch, y=yaw, z=roll)
	const float cx = cosf(pitch_radians / 2.f);
	const float sx = sinf(pitch_radians / 2.f);
	const float cy = cosf(yaw_radians / 2.f);
	const float sy = sinf(yaw_radians / 2.f);
	const float cz = cosf(roll_radians / 2.f);
	const float sz = sinf(roll_radians / 2.f);
	Eigen::Quaternionf q(
		cx*cy*cz + sx*sy*sz,
		sx*cy*cz - cx*sy*sz,
		cx*sy*cz + sx*cy*sz,
		cx*cy*sz - sx*sy*cz);

	return q;
}

void
psmove_quaternion_get_yaw_pitch_roll(
const Eigen::Quaternionf &q, float *out_yaw_radians, float *out_pitch_radians, float *out_roll_radians)
{
	float test = q.x()*q.y() + q.z()*q.w();

	if (test > 0.499f)
	{
		// singularity at north pole
		*out_yaw_radians = 2.f * atan2f(q.x(), q.w());
		*out_roll_radians = k_real_pi / 2.f;
		*out_pitch_radians = 0.f;
	}
	else if (test < -0.499f)
	{
		// singularity at south pole
		*out_yaw_radians = -2.f * atan2f(q.x(), q.w());
		*out_roll_radians = -k_real_pi / 2.f;
		*out_pitch_radians = 0.f;
	}
	else
	{
		float sqx = q.x()*q.x();
		float sqy = q.y()*q.y();
		float sqz = q.z()*q.z();

		*out_yaw_radians = atan2f(2.f*q.y()*q.w() - 2.f*q.x()*q.z(), 1.f - 2.f*sqy - 2.f*sqz);
		*out_roll_radians = asinf(2.f*test);
		*out_pitch_radians = atan2f(2.f*q.x()*q.w() - 2.f*q.y()*q.z(), 1.f - 2.f*sqx - 2.f*sqz);
	}
}

// Creates a quaternion that rotates clockwise about the axis for a positive angle
// when appied with psmove_vector_clockwise_rotate()
Eigen::Quaternionf
psmove_quaternion_angle_axis(float radians, const Eigen::Vector3f &axis)
{
	return Eigen::Quaternionf(Eigen::AngleAxisf(radians, axis));
}

Eigen::Quaternionf
psmove_quaternion_normalized_lerp(const Eigen::Quaternionf &a, const Eigen::Quaternionf &b, const float u)
{	
	Eigen::Quaternionf q(a.coeffs()*(1.f - u) + b.coeffs()*u);
	q.normalize();

	return q;
}

Eigen::Quaternionf
psmove_quaternion_safe_divide_with_default(const Eigen::Quaternionf &q, const float divisor, const Eigen::Quaternionf &default_result)
{
	Eigen::Quaternionf q_n;

	if (!is_nearly_zero(divisor))
	{
		q_n = Eigen::Quaternionf(q.coeffs() / divisor);
	}
	else
	{
		q_n = default_result;
	}

	return q_n;
}

float
psmove_quaternion_normalize_with_default(Eigen::Quaternionf &inout_v, const Eigen::Quaternionf &default_result)
{
	const float magnitude = inout_v.norm();
	inout_v = psmove_quaternion_safe_divide_with_default(inout_v, magnitude, default_result);
	return magnitude;
}

bool
psmove_quaternion_is_valid(const Eigen::Quaternionf &q)
{
	return is_valid_float(q.x()) && is_valid_float(q.y()) && is_valid_float(q.z()) && is_valid_float(q.w());
}

Eigen::Vector3f
psmove_vector3f_clockwise_rotate(const Eigen::Quaternionf &q, const Eigen::Vector3f &v)
{
	assert_quaternion_is_normalized(q);

	// Eigen rotates counterclockwise (i.e. q*v*q^-1), 
	// while we want the inverse of that (q^-1*v*q)
	return q.conjugate()._transformVector(v);
}

Eigen::Matrix3f
psmove_quaternion_to_clockwise_matrix3f(const Eigen::Quaternionf &q)
{
	return q.conjugate().toRotationMatrix();
}

Eigen::Quaternionf
psmove_matrix3f_to_clockwise_quaternion(const Eigen::Matrix3f &m)
{
	Eigen::Quaternionf q(m);

	return q.conjugate();
}