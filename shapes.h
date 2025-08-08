#ifndef SHAPES_H
#define SHAPES_H

#include "raytracing.h"
#include "hittable.h"

class sphere : public hittable {
  public:
     sphere(const point3& center, double radius, shared_ptr<material> mat)
      : center(center), radius(std::fmax(0,radius)), mat(mat) {}
      
      bool hit(const ray& r, interval ray_t, hit_record& rec) const override {        
        vec3 oc = center - r.origin();
        auto a = r.direction().length_squared();
        auto h = dot(r.direction(), oc);
        auto c = oc.length_squared() - radius*radius;

        auto discriminant = h*h - a*c;
        if (discriminant < 0)
            return false;

        auto sqrtd = std::sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        auto root = (h - sqrtd) / a;
        if (!ray_t.surrounds(root)) {
            root = (h + sqrtd) / a;
            if (!ray_t.surrounds(root))
                return false;
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        vec3 outward_normal = (rec.p - center) / radius;
        rec.set_face_normal(r, outward_normal);
        rec.mat = mat;

        return true;
    }

  private:
    point3 center;
    double radius;
    shared_ptr<material> mat;
};

class cube : public hittable {
  public:
     cube(const point3& center, double side_length, shared_ptr<material> mat)
      : center(center), side_length(std::fmax(0,side_length)), mat(mat) {}
      
      bool hit(const ray& r, interval ray_t, hit_record& rec) const override {        
        // Axis-aligned bounding box hit test
        vec3 min = center - vec3(side_length/2, side_length/2, side_length/2);
        vec3 max = center + vec3(side_length/2, side_length/2, side_length/2);

        double t_min = (min.x() - r.origin().x()) / r.direction().x();
        double t_max = (max.x() - r.origin().x()) / r.direction().x();
        if (t_min > t_max) std::swap(t_min, t_max);

        double ty_min = (min.y() - r.origin().y()) / r.direction().y();
        double ty_max = (max.y() - r.origin().y()) / r.direction().y();
        if (ty_min > ty_max) std::swap(ty_min, ty_max);

        if ((t_min > ty_max) || (ty_min > t_max))
            return false;

        if (ty_min > t_min)
            t_min = ty_min;
        if (ty_max < t_max)
            t_max = ty_max;

        double tz_min = (min.z() - r.origin().z()) / r.direction().z();
        double tz_max = (max.z() - r.origin().z()) / r.direction().z();
        if (tz_min > tz_max) std::swap(tz_min, tz_max);

        if ((t_min > tz_max) || (tz_min > t_max))
            return false;

        if (tz_min > t_min)
            t_min = tz_min;
        if (tz_max < t_max)
            t_max = tz_max;

        if (!ray_t.surrounds(t_min))
            return false;

        rec.t = t_min;
        rec.p = r.at(rec.t);
        
        // Determine the normal vector based on which face was hit
        vec3 outward_normal;
        if (std::fabs(rec.p.x() - min.x()) < 1e-8) {
            outward_normal = vec3(-1, 0, 0);
        } else if (std::fabs(rec.p.x() - max.x()) < 1e-8) {
            outward_normal = vec3(1, 0, 0);
        } else if (std::fabs(rec.p.y() - min.y()) < 1e-8) {
            outward_normal = vec3(0, -1, 0);
        } else if (std::fabs(rec.p.y() - max.y()) < 1e-8) {
            outward_normal = vec3(0, 1, 0);
        } else if (std::fabs(rec.p.z() - min.z()) < 1e-8) {
            outward_normal = vec3(0, 0, -1);
        } else if (std::fabs(rec.p.z() - max.z()) < 1e-8) {
            outward_normal = vec3(0, 0, 1);
        } else {
            return false; // Should not happen, but just in case
        }
        rec.set_face_normal(r, outward_normal);
        rec.mat = mat;

        return true;
    }

  private:
    point3 center;
    double side_length;
    shared_ptr<material> mat;
};

#endif