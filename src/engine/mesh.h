#ifndef SRC_ENGINE_MESH_H_
#define SRC_ENGINE_MESH_H_

#include <valarray>
#include <vector>
#include <array>
#include <memory>
#include <iostream>
#include <numeric>
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "background.h"

namespace Engine {
    class Mesh {

        std::valarray<double> position;
        std::vector<std::shared_ptr<Mesh>> children;

    public:

        enum Axis {

            AXIS_X,
            AXIS_Y,
            AXIS_Z

        };

        static constexpr long double
            EPSILON = 0.0000000000000001,
            PI = 3.141592653589793238462643383279502884L,
            DEG30 = PI / 6.0,
            DEG45 = PI / 4.0,
            DEG60 = PI / 3.0,
            DEG90 = PI / 2.0,
            DEG135 = DEG90 + DEG45,
            DEG225 = -DEG135,
            DEG270 = -DEG90,
            DEG315 = -DEG45;

        template <typename T>
        inline static bool zero (const std::valarray<T> &vec) {
            for (const auto &val : vec) {
                if (val) {
                    return false;
                }
            }
            return true;
        }

        inline static double dot (const std::valarray<double> &ray_1, const std::valarray<double> &ray_2) {
            return (ray_1 * ray_2).sum();
        }

        template <typename T>
        inline static T clamp (T value, T min_value, T max_value) {
            if (value > max_value) {
                return max_value;
            } else if (value < min_value) {
                return min_value;
            }
            return value;
        }

        inline static std::valarray<double> cross (const std::valarray<double> &ray_1, const std::valarray<double> &ray_2) {
            return {
                ray_1[1] * ray_2[2] - ray_2[1] * ray_1[2],
                ray_1[2] * ray_2[0] - ray_2[2] * ray_1[0],
                ray_1[0] * ray_2[1] - ray_2[0] * ray_1[1]
            };
        }

        inline static double norm2 (const std::valarray<double> &ray) {
            return dot(ray, ray);
        }

        inline static double norm (const std::valarray<double> &ray) {
            return std::sqrt(norm2(ray));
        }

        inline static std::valarray<double> resize (const std::valarray<double> &ray, const double vector_size, const double new_size) {
            return ray * (new_size / vector_size);
        }

        inline static std::valarray<double> resize (const std::valarray<double> &ray, const double new_size) {
            return resize(ray, norm(ray), new_size);
        }

        inline static double distance (const std::valarray<double> &point_1, const std::valarray<double> &point_2) {
            return norm(point_1 - point_2);
        }

        inline static std::valarray<double> rotate (const std::valarray<double> &ray, const double angle, const Axis axis = Axis::AXIS_Z) {

            if (angle == 0.0) {
                return ray;
            }

            const double
                sin_angle = std::sin(angle),
                cos_angle = std::cos(angle);

            switch (axis) {
                case Axis::AXIS_X:
                    return { ray[0], cos_angle * ray[1] - sin_angle * ray[2], sin_angle * ray[1] + cos_angle * ray[2] };
                case Axis::AXIS_Y:
                    return { cos_angle * ray[0] + sin_angle * ray[2], ray[1], cos_angle * ray[2] - sin_angle * ray[0] };
                default:
                    return { cos_angle * ray[0] - sin_angle * ray[1], sin_angle * ray[0] + cos_angle * ray[1], ray[2] };
            }
        }

        inline static std::array<std::valarray<double>, 2> parametricEquation (
            const std::valarray<double> ray_start,
            const std::valarray<double> ray_end
        ) { return { ray_start, ray_end - ray_start }; }

        static std::vector<std::valarray<double>> implicitEquation (
            const std::valarray<double> ray_start,
            const std::valarray<double> ray_end
        );

        inline static double areaTriangle2D (
            const std::valarray<double> &tri_point_1,
            const std::valarray<double> &tri_point_2,
            const std::valarray<double> &tri_point_3
        ) {
            return std::abs((
                tri_point_1[0] * (tri_point_2[1] - tri_point_3[1]) +
                tri_point_2[0] * (tri_point_3[1] - tri_point_1[1]) +
                tri_point_3[0] * (tri_point_1[1] - tri_point_2[1])
            ) * 0.5);
        }

        inline static double areaRectangle2D (
            const std::valarray<double> &rect_top_left,
            const std::valarray<double> &rect_bottom_left,
            const std::valarray<double> &rect_bottom_right
        ) {
            return norm(rect_bottom_left - rect_top_left) * norm(rect_bottom_right - rect_bottom_left);
        }

        inline static std::array<std::array<std::valarray<double>, 2>, 4> edgesRectangle2D (
            const std::valarray<double> &rect_top_left,
            const std::valarray<double> &rect_bottom_left,
            const std::valarray<double> &rect_bottom_right,
            const std::valarray<double> &rect_top_right
        ) {
            return {{
                { rect_top_left, rect_bottom_left },
                { rect_bottom_left, rect_bottom_right },
                { rect_bottom_right, rect_top_right },
                { rect_top_right, rect_top_left }
            }};
        }

        inline static std::array<std::array<std::valarray<double>, 2>, 3> edgesTriangle2D (
            const std::valarray<double> &tri_point_1,
            const std::valarray<double> &tri_point_2,
            const std::valarray<double> &tri_point_3
        ) {
            return {{
                { tri_point_1, tri_point_2 },
                { tri_point_2, tri_point_3 },
                { tri_point_3, tri_point_1 }
            }};
        }

        static double distanceRayToPoint (
            const std::valarray<double> &ray_start,
            const std::valarray<double> &ray_end,
            const std::valarray<double> &point,
            std::valarray<double> &near_point,
            bool infinite = false
        ) {
            const std::valarray<double> delta_ray = ray_end - ray_start;
            const double length_pow = norm2(delta_ray);
            double param;

            if (length_pow == 0.0) {
                return distance(point, ray_start);
            }

            param = ((point - ray_start) * delta_ray).sum() / length_pow;

            if (!infinite) {
                param = clamp(param, 0.0, 1.0);
            }

            near_point = ray_start + param * delta_ray;

            return distance(point, near_point);
        }

        static bool collisionPointTriangle2D (
            const std::valarray<double> &point,
            const std::valarray<double> &tri_point_1,
            const std::valarray<double> &tri_point_2,
            const std::valarray<double> &tri_point_3
        );

        static bool collisionPointRectangle2D (
            const std::valarray<double> &point,
            const std::valarray<double> &rect_top_left,
            const std::valarray<double> &rect_bottom_left,
            const std::valarray<double> &rect_bottom_right,
            const std::valarray<double> &rect_top_right
        );

        inline static bool collisionSpheres (
            std::valarray<double> position_1,
            double radius_1,
            std::valarray<double> position_2,
            double radius_2
        ) { return distance(position_1, position_2) <= (radius_1 + radius_2); }

        static double distanceRays (
            const std::valarray<double> &ray_1_start,
            const std::valarray<double> &ray_1_end,
            const std::valarray<double> &ray_2_start,
            const std::valarray<double> &ray_2_end,
            std::valarray<double> &ray_start,
            std::valarray<double> &ray_end
        );

        static bool collisionRectangles2D (
            const std::valarray<double> &rect_1_top_left,
            const std::valarray<double> &rect_1_bottom_left,
            const std::valarray<double> &rect_1_bottom_right,
            const std::valarray<double> &rect_1_top_right,
            const std::valarray<double> &rect_2_top_left,
            const std::valarray<double> &rect_2_bottom_left,
            const std::valarray<double> &rect_2_bottom_right,
            const std::valarray<double> &rect_2_top_right,
            std::valarray<double> &near_point
        );

        inline static bool collisionRaySphere (
            std::valarray<double> ray_start,
            std::valarray<double> ray_end,
            std::valarray<double> circle_center,
            double circle_radius,
            std::valarray<double> &near_point,
            bool infinite = false
        ) { return distanceRayToPoint(ray_start, ray_end, circle_center, near_point, infinite) <= circle_radius; }

        inline static bool collisionRectangleCircle2D (
            const std::valarray<double> &rect_top_left,
            const std::valarray<double> &rect_bottom_left,
            const std::valarray<double> &rect_bottom_right,
            const std::valarray<double> &rect_top_right,
            std::valarray<double> circle_center,
            double circle_radius,
            std::valarray<double> &near_point
        ) {

            if (collisionRaySphere(rect_top_left, rect_top_right, circle_center, circle_radius, near_point) ||
                collisionRaySphere(rect_top_right, rect_bottom_right, circle_center, circle_radius, near_point) ||
                collisionRaySphere(rect_bottom_left, rect_bottom_right, circle_center, circle_radius, near_point) ||
                collisionRaySphere(rect_top_left, rect_bottom_left, circle_center, circle_radius, near_point)) {
                return true;
            }

            if (collisionPointRectangle2D(rect_top_left, rect_bottom_left, rect_bottom_right, rect_top_right, circle_center)) {
                near_point = circle_center;
                return true;
            }

            return false;
        }

        Mesh (const std::valarray<double> &_position = { 0.0, 0.0, 0.0 }) :
            position(_position) {};

        virtual ~Mesh () {}

        void draw (const std::valarray<double> &offset, const Background *background, bool only_border = false) const {

            this->_draw(offset, background, only_border);

            if (!this->children.empty()) {
                std::valarray<double> position = offset + this->getPosition();

                for (const auto &mesh : this->children) {
                    mesh->draw(position, background, only_border);
                }
            }
        }

        inline void addChild (std::shared_ptr<Mesh> child) { this->children.push_back(child); }

        inline const std::valarray<double> &getPosition () const { return this->position; }

        inline void setPosition (const std::valarray<double> &_position) { this->position = _position; }

        inline virtual void _draw (const std::valarray<double> &offset, const Background *background, bool only_border) const {}

        inline virtual Mesh *getCollisionSpace (const std::valarray<double> &speed) const { return nullptr; }

        inline virtual bool detectCollision (
            const Mesh *other,
            const std::valarray<double> &my_offset,
            const std::valarray<double> &my_speed,
            const std::valarray<double> &other_offset,
            const std::valarray<double> &other_speed,
            std::valarray<double> &point,
            const bool try_inverse = true
        ) const {

            if (!(zero(my_speed) && zero(other_speed))) {
                const std::valarray<double> stopped = { 0.0, 0.0, 0.0 };
                const std::unique_ptr<const Mesh> my_space(this->getCollisionSpace(my_speed));

                if (my_space) {

                    if (my_space->detectCollision(other, my_offset, stopped, other_offset, other_speed, point, try_inverse)) {
                        std::cout << "first case " << this->getType() << " " << other->getType() << std::endl;
                        return true;
                    }

                    const std::unique_ptr<const Mesh> other_space(other->getCollisionSpace(other_speed));

                    if (
                        other_space &&
                        my_space->detectCollision(other_space.get(), my_offset, stopped, other_offset, stopped, point, try_inverse)
                    ) {
                        std::cout << "second case " << this->getType() << " " << other->getType() << std::endl;
                        return true;
                    }
                }
            }

            if (try_inverse) {
                return other->detectCollision(this, other_offset, other_speed, my_offset, my_speed, point, false);
            }
            return false;
        }

        inline virtual std::string getType () const { return "mesh"; }
    };

    class Rectangle2D : public Mesh {

        double width, height, angle;

        std::valarray<double> top_right, bottom_left, bottom_right;

    public:
        inline Rectangle2D (const std::valarray<double> &_position, double _width, double _height, double _angle = 0.0) :
            Mesh(_position), width(_width), height(_height), angle(_angle) {
            this->updatePositions();
        }

        void updatePositions (void) {
            const double
                height_angle = this->getAngle() - DEG90;

            const std::valarray<double>
                delta_height = { this->getHeight() * std::cos(height_angle), this->getHeight() * std::sin(height_angle), 0.0 };

            this->top_right = this->getTopLeftPosition() + std::valarray<double>(
                { this->getWidth() * std::cos(this->getAngle()), this->getWidth() * std::sin(this->getAngle()), 0.0 }
            );

            this->bottom_left = this->getTopLeftPosition() + delta_height;
            this->bottom_right = this->getTopRightPosition() + delta_height;

        }

        double getWidth (void) const { return this->width; }
        double getHeight (void) const { return this->height; }
        double getAngle (void) const { return this->angle; }

        inline void setWidth (const double _width) { this->width = _width, this->updatePositions(); }
        inline void setHeight (const double _height) { this->height = _height, this->updatePositions(); }
        inline void setAngle (const double _angle) { this->angle = _angle, this->updatePositions(); }

        inline void setPosition (const std::valarray<double> &_position) { Mesh::setPosition(_position), this->updatePositions(); }

        inline const std::valarray<double> &getTopLeftPosition (void) const { return this->getPosition(); }
        inline const std::valarray<double> &getTopRightPosition (void) const { return this->top_right; }
        inline const std::valarray<double> &getBottomLeftPosition (void) const { return this->bottom_left; }
        inline const std::valarray<double> &getBottomRightPosition (void) const { return this->bottom_right; }

        void _draw (const std::valarray<double> &offset, const Background *background, const bool only_border) const {

            const std::valarray<double>
                top_left = this->getTopLeftPosition() + offset,
                bottom_left = this->getBottomLeftPosition() + offset,
                bottom_right = this->getBottomRightPosition() + offset,
                top_right = this->getTopRightPosition() + offset;

            if (only_border) {
                glBegin(GL_LINE_LOOP);
            } else {
                glBegin(GL_TRIANGLE_FAN);
            }

            background->apply();

            glVertex3d(top_left[0], top_left[1], top_left[2]);
            glVertex3d(bottom_left[0], bottom_left[1], bottom_left[2]);
            glVertex3d(bottom_right[0], bottom_right[1], bottom_right[2]);
            glVertex3d(top_right[0], top_right[1], top_right[2]);

            glEnd();
        }

        bool detectCollision (
            const Mesh *other,
            const std::valarray<double> &my_offset,
            const std::valarray<double> &my_speed,
            const std::valarray<double> &other_offset,
            const std::valarray<double> &other_speed,
            std::valarray<double> &point,
            const bool try_inverse = true
        ) const {

            if (other->getType() == "rectangle2d") {
                const Rectangle2D *rect = static_cast<const Rectangle2D *>(other);
                if (Mesh::collisionRectangles2D(
                    my_offset + this->getTopLeftPosition(),
                    my_offset + this->getBottomLeftPosition(),
                    my_offset + this->getBottomRightPosition(),
                    my_offset + this->getTopRightPosition(),
                    other_offset + rect->getTopLeftPosition(),
                    other_offset + rect->getBottomLeftPosition(),
                    other_offset + rect->getBottomRightPosition(),
                    other_offset + rect->getTopRightPosition(),
                    point
                )) {
                    return true;
                }
            }
            return Mesh::detectCollision(other, my_offset, my_speed, other_offset, other_speed, point, try_inverse);
        }

        inline std::string getType (void) const { return "rectangle2d"; }

    };

    class Polygon2D : public Mesh {

        double radius, angle;
        int sides;

    public:

        inline Polygon2D (const std::valarray<double> &_position, double _radius, int _sides, double _angle = 0.0) :
            Mesh(_position), radius(_radius), angle(_angle), sides(_sides) {};

        void _draw (const std::valarray<double> &offset, const Background *background, bool only_border) const {

            std::valarray<double> position = offset + this->getPosition();

            const double step = (Polygon2D::PI * 2.0) / static_cast<double>(this->sides);
            if (only_border) {
                glBegin(GL_LINE_LOOP);
            } else {
                glBegin(GL_TRIANGLE_FAN);
            }

            background->apply();

            for (int i = 0; i < this->sides; i++) {

                const double ang = i * step + this->getAngle();
                glVertex3d(position[0] + this->getRadius() * std::cos(ang), position[1] + this->getRadius() * std::sin(ang), position[2]);

            }

            glEnd();

        }

        inline double getRadius (void) const { return this->radius; }
        inline double getAngle (void) const { return this->angle; }

        inline void setRadius (const double _radius) { this->radius = _radius; }

        Mesh *getCollisionSpace (const std::valarray<double> &speed) const {

            const double angle = std::atan2(speed[1], speed[0]);

            const std::valarray<double>
                difference = {
                    this->getRadius() * static_cast<double>(std::cos(angle + DEG90)),
                    this->getRadius() * static_cast<double>(std::sin(angle + DEG90)),
                    0.0
                },
                top_position = this->getPosition() + difference;

            return new Rectangle2D(top_position, Mesh::norm(speed), this->getRadius() * 2.0, angle);
        }

        bool detectCollision (
            const Mesh *other,
            const std::valarray<double> &my_offset,
            const std::valarray<double> &my_speed,
            const std::valarray<double> &other_offset,
            const std::valarray<double> &other_speed,
            std::valarray<double> &point,
            const bool try_inverse = true
        ) const {

            if (other->getType() == "polygon2d" || other->getType() == "sphere2d") {
                const Polygon2D *poly = static_cast<const Polygon2D *>(other);
                point = (this->getPosition() + other->getPosition()) * 0.5;
                if (Mesh::collisionSpheres(my_offset + this->getPosition(), this->getRadius(), other_offset + other->getPosition(), poly->getRadius())) {
                    return true;
                }
            } else if (other->getType() == "rectangle2d") {
                const Rectangle2D *rect = static_cast<const Rectangle2D *>(other);
                if (Mesh::collisionRectangleCircle2D(
                    other_offset + rect->getTopLeftPosition(),
                    other_offset + rect->getBottomLeftPosition(),
                    other_offset + rect->getBottomRightPosition(),
                    other_offset + rect->getTopRightPosition(),
                    my_offset + this->getPosition(),
                    this->getRadius(),
                    point
                )) {
                    return true;
                }
            }
            return Mesh::detectCollision(other, my_offset, my_speed, other_offset, other_speed, point, try_inverse);
        }

        inline virtual std::string getType () const { return "polygon2d"; }

    };

    class Sphere2D : public Polygon2D {
    public:
        Sphere2D (const std::valarray<double> &_position, const double _radius) :
            Polygon2D (_position, _radius, 100, 0.0) {}

        inline virtual std::string getType () const { return "sphere2d"; }
    };
};

#endif
