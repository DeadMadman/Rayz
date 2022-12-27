//refs
//https://raytracing.github.io/books/RayTracingInOneWeekend.html

#include "camera.h"
#include "color.h"
#include "hittableList.h"
#include "sphere.h"

#include "movingSphere.h"

#include "box.h"
#include "material.h"
#include "rectXY.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

hittableList cornellBox() {
    hittableList objects;

    auto red   = make_shared<lambertian>(color(0.65f, 0.05f, 0.05f));
    auto white = make_shared<lambertian>(color(0.73f, 0.73f, 0.73f));
    auto green = make_shared<lambertian>(color(0.12f, 0.45f, 0.15f));
    auto light = make_shared<diffuseLight>(color(15, 15, 15));

    objects.add(make_shared<rectYZ>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<rectYZ>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<rectXZ>(213, 343, 227, 332, 554, light));
    objects.add(make_shared<rectXZ>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<rectXZ>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<rectXY>(0, 555, 0, 555, 555, white));

    shared_ptr<hittable> box1 = make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white);
    box1 = make_shared<rotateY>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265,0,295));
    objects.add(box1);

    shared_ptr<hittable> box2 = make_shared<box>(point3(0,0,0), point3(165,165,165), white);
    box2 = make_shared<rotateY>(box2, -18);
    box2 = make_shared<translate>(box2, vec3(130,0,65));
    objects.add(box2);
    return objects;
}

void drawSmallSpheres(hittableList& world) {
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto chooseMat = randFloat();
            point3 center(a + 0.9f * randFloat(), 0.2f, b + 0.9f * randFloat());

            if ((center - point3(4, 0.2f, 0)).length() > 0.9f) {
                shared_ptr<material> sphereMaterial;

                if (chooseMat < 0.8f) {
                    // diffuse
                    auto albedo = color::random() * color::random();
                    sphereMaterial = make_shared<lambertian>(albedo);
                    auto center2 = center + vec3(0, randFloat(0,.5f), 0);
                    world.add(make_shared<movingSphere>(center, center2, 0.0f, 1.0f, 0.2f, sphereMaterial));
                
                } else if (chooseMat < 0.95f) {
                    // metal
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = randFloat(0, 0.5);
                    sphereMaterial = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphereMaterial));
                } else {
                    // glass
                    sphereMaterial = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, 0.2, sphereMaterial));
                }
            }
        }
    }
}

hittableList randomScene() {
    // initialize random seed:
    srand (time(nullptr));
    
    hittableList world;
    
    auto checker = make_shared<checkerTexture>(color(0.2f, 0.3f, 0.1f), color(0.9f, 0.9f, 0.9f));
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(checker)));

    //auto groundMaterial = make_shared<lambertian>(color(0.2f, 0.2f, 0.2f));
    //world.add(make_shared<sphere>(point3(0,-1000,0), 1000, groundMaterial));

    drawSmallSpheres(world);
    
    auto dielectricTex = make_shared<dielectric>(1.5);
    auto lembertianTex = make_shared<lambertian>(color(0.4f, 0.7f, 0.1f));
    auto metalTex = make_shared<metal>(color(0.7f, 0.6f, 0.5f), 0.0);

    auto checkeredTex = make_shared<lambertian>(checker);
    auto noise = make_shared<noiseTexture>(1);
    auto perlinTex = make_shared<lambertian>(noise);
    
    auto earthTexture = make_shared<imageTexture>("earthmap.jpg");
    auto earthSurface = make_shared<lambertian>(earthTexture);
    
    auto difflight = make_shared<diffuseLight>(color(4,4,4));
    
    world.add(make_shared<sphere>(point3(4, 1.0f, 0.5f), 1.0, dielectricTex));
    world.add(make_shared<sphere>(point3(2, 1.0f, -1.5f), 1.0, lembertianTex));
    world.add(make_shared<sphere>(point3(0, 1.0f, 2.5f), 1.0, metalTex));
    
    world.add(make_shared<sphere>(point3(5, 0.5f, -1.0f), 0.5, checkeredTex));
    world.add(make_shared<sphere>(point3(4.0f, 0.5f, 3.0f), 0.5, perlinTex));
    world.add(make_shared<sphere>(point3(2, 0.5f, 2.0f), 0.5f, earthSurface));

    //world.add(make_shared<sphere>(point3(4, 2.0f, 2.0f), 0.5f, difflight));
    return world;
}

color rayColor(const ray& r, const color& background, const hittable& world, int depth) {
    hitRecord rec;
    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return color(0,0,0);
    
    if (!world.hit(r, 0.001f, infinity, rec)) {
        return background;
    }

    ray scattered;
    color attenuation;
    color emitted = rec.matPtr->emitted(rec.u, rec.v, rec.p);
        
    if (!rec.matPtr->scatter(r, rec, attenuation, scattered))
        return emitted;
    return emitted + attenuation * rayColor(scattered, background, world, depth-1);
}

int main()
{
    // Image
    const auto aspectRatio = 16.0f / 9.0f;
    const int imageWidth = 400;
    const int imageHeight = static_cast<int>(imageWidth / aspectRatio);
    const int samplesPerPixel = 200;
    const int maxDepth = 50;
    
    // World
    color background(1.0f, 1.0f, 0.6f);
    auto world = randomScene();
    
    // Camera
    point3 lookfrom(13,2,3);
    point3 lookat(0,0,0);
    
    vec3 vup(0,1,0);
    auto distToFocus = 10.0f;
    auto aperture = 0.1f;
    float vfov = 20.0;

    int sceneNr = 1;
    switch (sceneNr) {
    case 1:
        background = {0.0f, 0.0f, 0.0f};
        world = cornellBox();
        vfov = 40;
        lookfrom = point3(278, 278, -800);
        lookat = point3(278, 278, 0);
    break;
        default:
    break;
    }
    
    camera cam(lookfrom, lookat, vup, vfov, aspectRatio, aperture, distToFocus, 0.0f, 1.0f);
    
    // Render
    std::string lines;

    float widthFac = 1.0f / static_cast<float>(imageWidth - 1);
    float heightFac = 1.0f / static_cast<float>(imageHeight - 1);
    
    static RGB data[imageHeight][imageWidth];
    
    for (int j = imageHeight - 1 ; j >= 0; --j) {
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < imageWidth; ++i) {
            color pixelColor(0, 0, 0);
            
            float fi = static_cast<float>(i);
            float fj = imageHeight - static_cast<float>(j);
            
            for (int s = 0; s < samplesPerPixel; s++) {
                float u0 = (fi + randFloat()) * widthFac;
                float v0 = (fj + randFloat()) * heightFac;
                ray r0 = cam.get_ray(u0, v0);
                pixelColor += rayColor(r0, background, world, maxDepth);
            }
            writeColor(data[j][i], pixelColor, samplesPerPixel);
        }
    }
    stbi_write_bmp("image1.bmp", imageWidth , imageHeight, sizeof(RGB), data);
    stbi_write_jpg("image1.jpg", imageWidth , imageHeight, sizeof(RGB), data, 100);
 
    std::cerr << "\nDone.\n";
}
