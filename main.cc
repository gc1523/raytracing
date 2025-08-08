#include "raytracing.h"

#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "shapes.h"
#include "material.h"
#include <random>
#include <fstream>

int image_generation(unsigned int seed = 69, point3 lookfrom = point3(13,3,3), point3 lookat = point3(0,1,0), const std::string& filename = "output.ppm") {
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Failed to open " << filename << " for writing.\n";
        return 1;
    }

    std::mt19937 rng(seed);

    hittable_list world;

    auto ground_material = make_shared<lambertian>(colour(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double(rng);
            point3 center(a + 0.9*random_double(rng), 0.2, b + 0.9*random_double(rng));

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = colour::random(rng) * colour::random(rng);
                    material = make_shared<lambertian>(albedo);
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = colour::random(0.5, 1, rng);
                    auto fuzz = random_double(0, 0.5, rng);
                    material = make_shared<metal>(albedo, fuzz);
                } else {
                    // glass
                    material = make_shared<dielectric>(1.5);
                }
                bool choose_shape = a * a * b * b % 2;
                if (choose_shape < 0.5) {
                    world.add(make_shared<sphere>(center, 0.2, material));
                } else {
                    world.add(make_shared<cube>(center, 0.4, material));    
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(colour(0.4, 0.2, 0.1));
    world.add(make_shared<cube>(point3(-4, 1, 0), 2.0, material2));

    auto material3 = make_shared<metal>(colour(0.6, 0.6, 0.6), 0.05);
    world.add(make_shared<sphere>(point3(0, 1, 0), 2.0, material3));

    // Add a green metal sphere just behind the camera, this makes the "camera"
    // appear in reflections.

    point3 green_sphere_center(lookfrom.x() + (lookfrom.x() - lookat.x()) * 0.5,
                               lookfrom.y() + (lookfrom.y() - lookat.y()) * 0.5,
                               lookfrom.z() + (lookfrom.z() - lookat.z()) * 0.5);

    auto material4 = make_shared<metal>(colour(0, 1, 0), 0);
    world.add(make_shared<sphere>(green_sphere_center, 0.25, material4));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 3840;
    cam.samples_per_pixel = 50;
    cam.max_depth         = 50;

    cam.vfov     = 20;
    cam.lookfrom = lookfrom;
    cam.lookat   = lookat;
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0.6;
    cam.focus_dist    = 10.0;

    cam.render(world, seed, out);
    return 0;
}

void video_generation() {
    std::vector<std::string> png_frames;

    int frame_idx = 0;
    // Rotate around the scene
    for (int i = 0; i < 360; i += 3) {
        double angle = i * M_PI / 180.0;
        point3 lookfrom(13 * std::sin(angle), 3, 13 * std::cos(angle));
        point3 lookat(0, 1, 0);
        char ppm_name[64]; char png_name[64];
        std::sprintf(ppm_name, "generation/frame_%04d.ppm", frame_idx);
        std::sprintf(png_name, "generation/frame_%04d.png", frame_idx);
        image_generation(42, lookfrom, lookat, ppm_name);

        std::string cmd = "convert " + std::string(ppm_name) + " " + png_name;
        std::system(cmd.c_str());
        png_frames.push_back(png_name);
        std::clog << "\rFrame " << frame_idx << " generated." << std::flush;
        frame_idx++;
    }

    // Generate frames moving near the metal sphere
    // This basically moves the camera along the line joining the centre of the 
    // metal sphere and the initial camera lookat point.
    // But we dont want to collide with the sphere, so we only move 80% of the 
    // distance between the two points.
    for (int i = 0; i < 40; i++) {
        double x = 0;
        double y = 3 - i / 50.0 * (3 - 1); 
        double z = 13 - i / 50.0 * (13 - 0);

        point3 lookfrom(x, y, z);
        point3 lookat(0, 1, 0);
        
        char ppm_name[64]; char png_name[64];
        std::sprintf(ppm_name, "generation/frame_%04d.ppm", frame_idx);
        std::sprintf(png_name, "generation/frame_%04d.png", frame_idx);
        image_generation(42, lookfrom, lookat, ppm_name);

        std::string cmd = "convert " + std::string(ppm_name) + " " + png_name;
        std::system(cmd.c_str());
        png_frames.push_back(png_name);

        std::clog << "\rFrame " << frame_idx << " generated." << std::flush;
        frame_idx++;
    }

    // Make our way back to the initial position for a smooth loop
    for (int i = 39; i >= 0; i--) {
        double x = 0;
        double y = 3 - i / 50.0 * (3 - 1); 
        double z = 13 - i / 50.0 * (13 - 0);

        point3 lookfrom(x, y, z);
        point3 lookat(0, 1, 0);
        
        char ppm_name[64]; char png_name[64];
        std::sprintf(ppm_name, "generation/frame_%04d.ppm", frame_idx);
        std::sprintf(png_name, "generation/frame_%04d.png", frame_idx);
        image_generation(42, lookfrom, lookat, ppm_name);

        std::string cmd = "convert " + std::string(ppm_name) + " " + png_name;
        std::system(cmd.c_str());
        png_frames.push_back(png_name);
        
        std::clog << "\rFrame " << frame_idx << " generated." << std::flush;
        frame_idx++;
    }

    std::system("ffmpeg -framerate 20 -i generation/frame_%04d.png -c:v libx264 -pix_fmt yuv420p videos/greenie.mp4");
}

int main() {
    image_generation(0, point3(5,3,13), point3(0,1,0), "cube_reflection.ppm");
    // video_generation();
    return 0;
}

