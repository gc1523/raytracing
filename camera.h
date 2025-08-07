#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "material.h"
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <random>

class camera {
  public:
    double aspect_ratio = 1.0;       // Ratio of image width over height
    int    image_width  = 100;       // Rendered image width in pixel count
    int    samples_per_pixel = 10;   // Count of random samples for each pixel
    int    max_depth         = 10;   // Maximum number of ray bounces into scene

    double vfov = 90;                  // Vertical view angle (field of view)
    point3 lookfrom = point3(0,0,0);   // Point camera is looking from
    point3 lookat   = point3(0,0,-1);  // Point camera is looking at
    vec3   vup      = vec3(0,1,0);     // Camera-relative "up" direction

    double defocus_angle = 0;  // Variation angle of rays through each pixel
    double focus_dist = 10;    // Distance from camera lookfrom point to plane of perfect focus


    void render(const hittable& world, unsigned int seed, std::ostream& out) {
        initialize();

        std::vector<std::vector<colour>> framebuffer(image_height, std::vector<colour>(image_width));
        int thread_count = std::thread::hardware_concurrency();
        if (thread_count == 0) thread_count = 4; 

        std::atomic<int> lines_done{0};

        auto render_scanlines = [&](int start, int end, int thread_id) {
            std::mt19937 rng(seed + thread_id); // Unique generator per thread
            for (int j = start; j < end; ++j) {
                for (int i = 0; i < image_width; ++i) {
                    colour pixel_color(0,0,0);
                    for (int sample = 0; sample < samples_per_pixel; ++sample) {
                        ray r = get_ray(i, j, rng);
                        pixel_color += ray_colour(r, max_depth, world, rng);
                    }
                    framebuffer[j][i] = pixel_samples_scale * pixel_color;
                }
                int done = ++lines_done;
                if (done % 10 == 0 || done == image_height) {
                    std::clog << "\rScanlines remaining: " << (image_height - done) << "   " << std::flush;
                }
            }
        };

        std::vector<std::thread> threads;
        int lines_per_thread = image_height / thread_count;
        int extra = image_height % thread_count;
        int start = 0;
        for (int t = 0; t < thread_count; ++t) {
            int end = start + lines_per_thread + (t < extra ? 1 : 0);
            threads.emplace_back(render_scanlines, start, end, t);
            start = end;
        }
        for (auto& th : threads) th.join();

        out << "P3\n" << image_width << ' ' << image_height << "\n255\n";
        for (int j = 0; j < image_height; ++j) {
            for (int i = 0; i < image_width; ++i) {
                write_colour(out, framebuffer[j][i]);
            }
        }
        std::clog << "\rDone.                 \n";
    }

  private:
    int    image_height;         // Rendered image height
    double pixel_samples_scale;  // Color scale factor for a sum of pixel samples
    point3 center;               // Camera center
    point3 pixel00_loc;          // Location of pixel 0, 0
    vec3   pixel_delta_u;        // Offset to pixel to the right
    vec3   pixel_delta_v;        // Offset to pixel below
    vec3   u, v, w;              // Camera frame basis vectors
    vec3   defocus_disk_u;       // Defocus disk horizontal radius
    vec3   defocus_disk_v;       // Defocus disk vertical radius

    void initialize() {
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        pixel_samples_scale = 1.0 / samples_per_pixel;
        center = lookfrom;

        // Determine viewport dimensions.
        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta/2);
        auto viewport_height = 2 * h * focus_dist;
        auto viewport_width = viewport_height * (double(image_width)/image_height);

        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        vec3 viewport_u = viewport_width * u;    // Vector across viewport horizontal edge
        vec3 viewport_v = viewport_height * -v;  // Vector down viewport vertical edge

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Calculate the location of the upper left pixel.
        auto viewport_upper_left 
            = center - (focus_dist * w) - viewport_u/2 - viewport_v/2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // Calculate the camera defocus disk basis vectors.
        auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }

    ray get_ray(int i, int j, std::mt19937& rng) const {
        auto offset = sample_square(rng);
        auto pixel_sample = pixel00_loc
                          + ((i + offset.x()) * pixel_delta_u)
                          + ((j + offset.y()) * pixel_delta_v);

        auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample(rng);
        auto ray_direction = pixel_sample - ray_origin;

        return ray(ray_origin, ray_direction);
    }

    vec3 sample_square(std::mt19937& rng) const {
        return vec3(random_double(rng) - 0.5, random_double(rng) - 0.5, 0);
    }

    point3 defocus_disk_sample(std::mt19937& rng) const {
        auto p = random_in_unit_disk(rng);
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

    colour ray_colour(const ray& r, int depth, const hittable& world, std::mt19937& rng) const {
        // If we've exceeded the ray bounce limit, no more light is gathered.
        if (depth <= 0)
            return colour(0,0,0);

        hit_record rec;
        if (world.hit(r, interval(0.001, infinity), rec)) {
            ray scattered;
            colour attenuation;
            if (rec.mat->scatter(r, rec, attenuation, scattered, rng))
                return attenuation * ray_colour(scattered, depth-1, world, rng);
            return colour(0,0,0);
        }

        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5*(unit_direction.y() + 1.0);
        return (1.0-a)*colour(1.0, 1.0, 1.0) + a*colour(0.5, 0.7, 1.0);
    }
};

#endif