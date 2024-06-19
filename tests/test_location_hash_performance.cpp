#include "lochash/lochash.hpp"
#include "gtest/gtest.h"
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

using namespace lochash;

struct Sphere {
	float x, y, z;    // Position
	float vx, vy, vz; // Velocity
	float radius;
};

constexpr std::size_t precision = 16;
using LocationHash3D            = LocationHash<precision, float, Sphere>;

void update_position(Sphere & sphere, float delta_time)
{
	sphere.x += sphere.vx * delta_time;
	sphere.y += sphere.vy * delta_time;
	sphere.z += sphere.vz * delta_time;
}

void handle_collision(Sphere & a, Sphere & b)
{
	// Calculate the distance between the two spheres
	float dx       = b.x - a.x;
	float dy       = b.y - a.y;
	float dz       = b.z - a.z;
	float distance = std::sqrt(dx * dx + dy * dy + dz * dz);

	// If the spheres are colliding
	if (distance < a.radius + b.radius) {
		// Calculate the normal vector
		float nx = dx / distance;
		float ny = dy / distance;
		float nz = dz / distance;

		// Calculate the dot product of the velocity and normal vector
		float dot_product_a = a.vx * nx + a.vy * ny + a.vz * nz;
		float dot_product_b = b.vx * nx + b.vy * ny + b.vz * nz;

		// Reflect velocities along the normal vector
		a.vx -= 2 * dot_product_a * nx;
		a.vy -= 2 * dot_product_a * ny;
		a.vz -= 2 * dot_product_a * nz;

		b.vx -= 2 * dot_product_b * nx;
		b.vy -= 2 * dot_product_b * ny;
		b.vz -= 2 * dot_product_b * nz;
	}
}

void handle_boundary_collision(Sphere & sphere, float min_bound, float max_bound)
{
	float & x      = sphere.x;
	float & y      = sphere.y;
	float & z      = sphere.z;
	float & vx     = sphere.vx;
	float & vy     = sphere.vy;
	float & vz     = sphere.vz;
	float   radius = sphere.radius;

	if (x - radius < min_bound) {
		x = min_bound + radius;
		vx *= -1;
	} else if (x + radius > max_bound) {
		x = max_bound - radius;
		vx *= -1;
	}

	if (y - radius < min_bound) {
		y = min_bound + radius;
		vy *= -1;
	} else if (y + radius > max_bound) {
		y = max_bound - radius;
		vy *= -1;
	}

	if (z - radius < min_bound) {
		z = min_bound + radius;
		vz *= -1;
	} else if (z + radius > max_bound) {
		z = max_bound - radius;
		vz *= -1;
	}
}

std::vector<Sphere *> query_boundary_spheres(LocationHash3D & location_hash, float min_bound, float max_bound,
                                             float boundary_range)
{
	std::vector<Sphere *> boundary_spheres;

	auto add_spheres_near_boundary = [&](float coord, float radius) {
		auto results = location_hash.query(coord, coord, coord, radius);
		for (const auto & result : results) {
			boundary_spheres.push_back(result.second);
		}
	};

	// Check boundaries within the specified range
	add_spheres_near_boundary(min_bound, boundary_range);
	add_spheres_near_boundary(max_bound, boundary_range);

	return boundary_spheres;
}

TEST(test_location_hash_performance, StressTest)
{
	LocationHash3D      location_hash;
	std::vector<Sphere> spheres;

	// Initialize spheres with random positions and velocities
	for (int i = 0; i < 1000; ++i) {
		Sphere sphere = {static_cast<float>(rand() % 1000),   static_cast<float>(rand() % 1000),
		                 static_cast<float>(rand() % 1000),   static_cast<float>(rand() % 10 - 5),
		                 static_cast<float>(rand() % 10 - 5), static_cast<float>(rand() % 10 - 5),
		                 static_cast<float>(rand() % 10 + 1)};
		spheres.push_back(sphere);
		location_hash.add(&spheres.back(), sphere.x, sphere.y, sphere.z);
	}

	double total_update_time              = 0.0;
	double total_collision_detection_time = 0.0;
	double total_boundary_handling_time   = 0.0;

	double total_frame_time = 0.0;
	double min_frame_time   = std::numeric_limits<double>::max();
	double max_frame_time   = std::numeric_limits<double>::min();

	const float boundary_range = 10.0f; // Range to check for boundary collisions
	const int   num_frames     = 10;

	double min_update_time              = std::numeric_limits<double>::max();
	double max_update_time              = std::numeric_limits<double>::min();
	double min_boundary_handling_time   = std::numeric_limits<double>::max();
	double max_boundary_handling_time   = std::numeric_limits<double>::min();
	double min_collision_detection_time = std::numeric_limits<double>::max();
	double max_collision_detection_time = std::numeric_limits<double>::min();

	for (int frame = 0; frame < num_frames; ++frame) {
		auto frame_start_time = std::chrono::high_resolution_clock::now();

		// Clear and re-add all spheres to the location hash
		location_hash.clear();

		auto start_time = std::chrono::high_resolution_clock::now();
		for (auto & sphere : spheres) {
			update_position(sphere, 0.016f); // Using fixed delta time for position updates
		}
		auto   end_time    = std::chrono::high_resolution_clock::now();
		double update_time = std::chrono::duration<double, std::milli>(end_time - start_time).count();
		total_update_time += update_time;
		if (update_time < min_update_time) {
			min_update_time = update_time;
		}
		if (update_time > max_update_time) {
			max_update_time = update_time;
		}

		start_time            = std::chrono::high_resolution_clock::now();
		auto boundary_spheres = query_boundary_spheres(location_hash, 0.0f, 1000.0f, boundary_range);
		for (auto & sphere : boundary_spheres) {
			handle_boundary_collision(*sphere, 0.0f, 1000.0f);
			location_hash.add(sphere, sphere->x, sphere->y, sphere->z);
		}
		end_time                      = std::chrono::high_resolution_clock::now();
		double boundary_handling_time = std::chrono::duration<double, std::milli>(end_time - start_time).count();
		total_boundary_handling_time += boundary_handling_time;
		if (boundary_handling_time < min_boundary_handling_time) {
			min_boundary_handling_time = boundary_handling_time;
		}
		if (boundary_handling_time > max_boundary_handling_time) {
			max_boundary_handling_time = boundary_handling_time;
		}

		start_time = std::chrono::high_resolution_clock::now();
		for (auto & sphere : spheres) {
			auto results = location_hash.query(sphere.x, sphere.y, sphere.z, sphere.radius);
			for (const auto & result : results) {
				if (result.second != &sphere) {
					handle_collision(sphere, *result.second);
				}
			}
		}
		end_time                        = std::chrono::high_resolution_clock::now();
		double collision_detection_time = std::chrono::duration<double, std::milli>(end_time - start_time).count();
		total_collision_detection_time += collision_detection_time;
		if (collision_detection_time < min_collision_detection_time) {
			min_collision_detection_time = collision_detection_time;
		}
		if (collision_detection_time > max_collision_detection_time) {
			max_collision_detection_time = collision_detection_time;
		}

		auto   frame_end_time = std::chrono::high_resolution_clock::now();
		double frame_time     = std::chrono::duration<double, std::milli>(frame_end_time - frame_start_time).count();
		total_frame_time += frame_time;
		if (frame_time < min_frame_time) {
			min_frame_time = frame_time;
		}
		if (frame_time > max_frame_time) {
			max_frame_time = frame_time;
		}
	}

	double average_frame_time = total_frame_time / num_frames;
	double average_fps        = 1000.0 / average_frame_time; // Convert to FPS
	double min_fps            = 1000.0 / max_frame_time;
	double max_fps            = 1000.0 / min_frame_time;

	double average_update_time              = total_update_time / num_frames;
	double average_boundary_handling_time   = total_boundary_handling_time / num_frames;
	double average_collision_detection_time = total_collision_detection_time / num_frames;

	std::cout << "Total update position time: " << total_update_time << " milliseconds" << std::endl;
	std::cout << "Total boundary handling time: " << total_boundary_handling_time << " milliseconds" << std::endl;
	std::cout << "Total collision detection time: " << total_collision_detection_time << " milliseconds" << std::endl;

	std::cout << "Average FPS: " << average_fps << std::endl;
	std::cout << "Minimum FPS: " << min_fps << std::endl;
	std::cout << "Maximum FPS: " << max_fps << std::endl;

	std::cout << "Average update position time per frame: " << average_update_time << " milliseconds" << std::endl;
	std::cout << "Minimum update position time per frame: " << min_update_time << " milliseconds" << std::endl;
	std::cout << "Maximum update position time per frame: " << max_update_time << " milliseconds" << std::endl;

	std::cout << "Average boundary handling time per frame: " << average_boundary_handling_time << " milliseconds"
	          << std::endl;
	std::cout << "Minimum boundary handling time per frame: " << min_boundary_handling_time << " milliseconds"
	          << std::endl;
	std::cout << "Maximum boundary handling time per frame: " << max_boundary_handling_time << " milliseconds"
	          << std::endl;

	std::cout << "Average collision detection time per frame: " << average_collision_detection_time << " milliseconds"
	          << std::endl;
	std::cout << "Minimum collision detection time per frame: " << min_collision_detection_time << " milliseconds"
	          << std::endl;
	std::cout << "Maximum collision detection time per frame: " << max_collision_detection_time << " milliseconds"
	          << std::endl;
}
