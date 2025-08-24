#include "raytracing.h"

#include "bvh.h"
#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "shapes.h"
#include "material.h"
#include "texture.h"
#include "quad.h"
#include <random>
#include <fstream>

#define RAND_SEED 42

void cornell_box(point3 lookfrom = point3(278, 278, -800), point3 lookat = point3(278, 278, 0), const std::string&filename = "output.ppm") {
    hittable_list world;

    auto red   = make_shared<lambertian>(colour(.65, .05, .05));
    auto white = make_shared<lambertian>(colour(.73, .73, .73));
    auto green = make_shared<lambertian>(colour(.12, .45, .15));
    auto light = make_shared<diffuse_light>(colour(15, 15, 15));

    world.add(make_shared<quad>(point3(555,0,0), vec3(0,555,0), vec3(0,0,555), green));
    world.add(make_shared<quad>(point3(0,0,0), vec3(0,555,0), vec3(0,0,555), red));
    world.add(make_shared<quad>(point3(343, 554, 332), vec3(-130,0,0), vec3(0,0,-105), light));
    world.add(make_shared<quad>(point3(0,0,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(make_shared<quad>(point3(555,555,555), vec3(-555,0,0), vec3(0,0,-555), white));
    world.add(make_shared<quad>(point3(0,0,555), vec3(555,0,0), vec3(0,555,0), white));

    world.add(box(point3(130, 0, 65), point3(295, 165, 230), white));
    world.add(box(point3(265, 0, 295), point3(430, 330, 460), white));

    camera cam;

    cam.aspect_ratio      = 1.0;
    cam.image_width       = 1920;
    cam.samples_per_pixel = 400;
    cam.max_depth         = 200;
    cam.background        = colour(0,0,0);

    cam.vfov     = 40;
    cam.lookfrom = lookfrom;
    cam.lookat   = lookat;
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;
    std::ofstream out(filename);
    cam.render(world, RAND_SEED, out);
}


void simple_light(point3 lookfrom = point3(26,3,6), point3 lookat = point3(0,2,0), const std::string&filename = "output.ppm") {
    hittable_list world;
    std::ofstream out(filename);
    auto pertext = make_shared<noise_texture>(4);
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(pertext)));
    world.add(make_shared<sphere>(point3(0,2,0), 2, make_shared<lambertian>(pertext)));

    auto difflight = make_shared<diffuse_light>(colour(4,4,4));
    world.add(make_shared<quad>(point3(3,1,-2), vec3(2,0,0), vec3(0,2,0), difflight));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 1920;
    cam.samples_per_pixel = 250;
    cam.max_depth         = 125;
    cam.background        = colour(0,0,0);

    cam.vfov     = 20;
    cam.lookfrom = lookfrom;
    cam.lookat   = lookat;
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world, RAND_SEED, out);
}

void earth(point3 lookfrom = point3(13,3,3), point3 lookat = point3(0,0,0), const std::string&filename = "earth.ppm") {
    auto earth_texture = make_shared<image_texture>("textures/earthmap.jpg");
    auto earth_surface = make_shared<lambertian>(earth_texture);
    auto globe = make_shared<sphere>(point3(0,0,0), 2, earth_surface);

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 960;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.background        = colour(0.70, 0.80, 1.00);

    cam.vfov     = 20;
    cam.lookfrom = lookfrom;
    cam.lookat   = lookat;
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;
    std::ofstream out(filename);
    cam.render(hittable_list(globe, std::mt19937(RAND_SEED)), RAND_SEED, out);
}

void spinning_earth() {
    std::vector<std::string> png_frames;

    int frame_idx = 0;
    // Rotate around the scene
    for (int i = 0; i < 360; i += 3) {
        double angle = i * M_PI / 180.0;
        point3 lookfrom(13 * std::sin(angle), 3, 13 * std::cos(angle));
        point3 lookat(0, 0, 0);
        char ppm_name[64]; char png_name[64];
        std::sprintf(ppm_name, "generation/frame_%04d.ppm", frame_idx);
        std::sprintf(png_name, "generation/frame_%04d.png", frame_idx);
        earth(lookfrom, lookat, ppm_name);

        std::string cmd = "convert " + std::string(ppm_name) + " " + png_name;
        std::system(cmd.c_str());
        png_frames.push_back(png_name);
        std::clog << "\rFrame " << frame_idx << " generated." << std::flush;
        frame_idx++;
    }

    std::system("ffmpeg -framerate 20 -i generation/frame_%04d.png -c:v libx264 -pix_fmt yuv420p videos/video.mp4");

}

void perlin_spheres(unsigned int seed = RAND_SEED, point3 lookfrom = point3(13,3,3), point3 lookat = point3(0,1,0), const std::string& filename = "output.ppm") {
    hittable_list world;

    auto pertext = make_shared<noise_texture>(4);    
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(pertext)));
    world.add(make_shared<sphere>(point3(0,2,0), 2, make_shared<lambertian>(pertext)));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 1920;
    cam.samples_per_pixel = 500;
    cam.max_depth         = 500;
    cam.background        = colour(0.70, 0.80, 1.00);

    cam.vfov     = 20;
    cam.lookfrom = lookfrom;
    cam.lookat   = lookat;
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;
    std::ofstream out(filename);
    cam.render(world, seed, out);
}

int bouncing_spheres_image_generation(unsigned int seed = RAND_SEED, point3 lookfrom = point3(13,3,3), point3 lookat = point3(0,1,0), const std::string& filename = "output.ppm") {
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Failed to open " << filename << " for writing.\n";
        return 1;
    }

    std::mt19937 rng(seed);

    hittable_list world;
    auto checker = make_shared<checker_texture>(0.32, colour(.2, .3, .1), colour(.9, .9, .9));
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(checker)));

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
                    auto center2 = center + vec3(0, random_double(0.0, 0.5, rng), 0);
                    world.add(make_shared<sphere>(center, center2, 0.2, material));
                    continue;
                } else if (choose_mat < 0.9) {
                    // metal
                    auto albedo = colour::random(0.5, 1, rng);
                    auto fuzz = random_double(0, 0.5, rng);
                    material = make_shared<metal>(albedo, fuzz);
                    continue;
                } else {
                    // glass
                    material = make_shared<dielectric>(1.5);
                }
                world.add(make_shared<sphere>(center, 0.2, material));
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(colour(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(colour(0.6, 0.6, 0.6), 0.05);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material3));

    // Add a green metal sphere just behind the camera, this makes the "camera"
    // appear in reflections.

    point3 green_sphere_center(lookfrom.x() + (lookfrom.x() - lookat.x()) * 0.5,
                               lookfrom.y() + (lookfrom.y() - lookat.y()) * 0.5,
                               lookfrom.z() + (lookfrom.z() - lookat.z()) * 0.5);

    auto material4 = make_shared<metal>(colour(0, 1, 0), 0);
    world.add(make_shared<sphere>(green_sphere_center, 0.25, material4));

    world = hittable_list(make_shared<bvh_node>(world), rng);

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 1920; 
    cam.samples_per_pixel = 50;
    cam.max_depth         = 50;
    cam.background        = colour(0.70, 0.80, 1.00);

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
        // image_generation(42, lookfrom, lookat, ppm_name);

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
        // image_generation(42, lookfrom, lookat, ppm_name);

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
        // image_generation(42, lookfrom, lookat, ppm_name);

        std::string cmd = "convert " + std::string(ppm_name) + " " + png_name;
        std::system(cmd.c_str());
        png_frames.push_back(png_name);
        
        std::clog << "\rFrame " << frame_idx << " generated." << std::flush;
        frame_idx++;
    }

    std::system("ffmpeg -framerate 20 -i generation/frame_%04d.png -c:v libx264 -pix_fmt yuv420p videos/video.mp4");
}

void checkered_spheres(unsigned int seed = RAND_SEED, point3 lookfrom = point3(13,3,3), point3 lookat = point3(0,1,0), const std::string& filename = "output.ppm") {
    std::ofstream out(filename);

    hittable_list world;

    auto checker = make_shared<checker_texture>(0.32, colour(.2, .3, .1), colour(.9, .9, .9));

    world.add(make_shared<sphere>(point3(0,-10, 0), 10, make_shared<lambertian>(checker)));
    world.add(make_shared<sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.background        = colour(0.70, 0.80, 1.00);

    cam.vfov     = 20;
    cam.lookfrom = lookfrom;
    cam.lookat   = lookat;
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world, RAND_SEED, out);
}

void quads(const std::string& filename = "output.ppm") {
    hittable_list world;
    std::ofstream out(filename);
    // Materials
    auto left_red     = make_shared<lambertian>(colour(1.0, 0.2, 0.2));
    auto back_green   = make_shared<lambertian>(colour(0.2, 1.0, 0.2));
    auto right_blue   = make_shared<lambertian>(colour(0.2, 0.2, 1.0));
    auto upper_orange = make_shared<lambertian>(colour(1.0, 0.5, 0.0));
    auto lower_teal   = make_shared<lambertian>(colour(0.2, 0.8, 0.8));

    // Quads
    world.add(make_shared<quad>(point3(-3,-2, 5), vec3(0, 0,-4), vec3(0, 4, 0), left_red));
    world.add(make_shared<quad>(point3(-2,-2, 0), vec3(4, 0, 0), vec3(0, 4, 0), back_green));
    world.add(make_shared<quad>(point3( 3,-2, 1), vec3(0, 0, 4), vec3(0, 4, 0), right_blue));
    world.add(make_shared<quad>(point3(-2, 3, 1), vec3(4, 0, 0), vec3(0, 0, 4), upper_orange));
    world.add(make_shared<quad>(point3(-2,-3, 5), vec3(4, 0, 0), vec3(0, 0,-4), lower_teal));

    camera cam;

    cam.aspect_ratio      = 1.0;
    cam.image_width       = 1920;
    cam.samples_per_pixel = 250;
    cam.max_depth         = 100;
    cam.background        = colour(0.70, 0.80, 1.00);

    cam.vfov     = 80;
    cam.lookfrom = point3(0,0,9);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world, RAND_SEED, out);
}

int main() {
    switch (9) {
        case 1:  
            bouncing_spheres_image_generation();
            break;
        case 2:  
            video_generation();
            break;
        case 3:  
            checkered_spheres();  
            break;
        case 4:  
            earth();              
            break;
        case 5:  
            spinning_earth();
            break;
        case 6:  
            perlin_spheres();     
            break;
        case 7:  
            quads();
            break;
        case 8:
            simple_light();
            break;
        case 9:
            cornell_box();
            break;
    }
    return 0;
}

