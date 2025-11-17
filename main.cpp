#include <threepp/threepp.hpp>
#include <threepp/canvas/Canvas.hpp>
#include <iostream>
#include <threepp/loaders/AssimpLoader.hpp>
//include files
#include "carKeyController.hpp"
#include "trees.h"
#include "car.hpp"
#include "CollisionDetection.hpp"
#include "bears.hpp"
#include "GameObject.hpp"
#include "turret.hpp"


using namespace threepp;

//dette er første forsøk på å lage en independent branch

void updateCameraFollow(PerspectiveCamera& camera, Object3D& target, float offsetDistance = 15.0f, float offsetHeight = 3.0f, float smoothing = 0.1f) {
    static float smoothedRotation = 0.0f;

    // Smooth the rotation with exponential decay
    smoothedRotation += (target.rotation.y - smoothedRotation) * smoothing;

    float offsetX = offsetDistance * std::sin(smoothedRotation);
    float offsetZ = offsetDistance * std::cos(smoothedRotation);

    camera.position.x = target.position.x - offsetX;
    camera.position.y = target.position.y + offsetHeight;
    camera.position.z = target.position.z - offsetZ;

    camera.lookAt(target.position);
}

std::pair<float, float> randomPosition() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dist(0.f, 4000.f);

    return {dist(gen), dist(gen)};
}

std::pair<float, float> randomPosition2() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dist(0.f, 1.f);

    return {dist(gen), dist(gen)};
}


void updateBlurEffect(float& blurLevel, MeshBasicMaterial& material, float dt, float elapsedTime,
                      float maxBlur, float decayRate) {
    blurLevel = std::max(0.0f, blurLevel - decayRate * dt);

    if (blurLevel <= 0.0f) {
        material.opacity = 0.0f;
        return;
    }

    float intensity = blurLevel / maxBlur;

    if (blurLevel <= 2.0f) {
        material.opacity = intensity * 0.3f;
    } else if (blurLevel <= 5.0f) {
        material.opacity = 0.3f + (intensity - 0.25f) * 0.4f;
    } else {
        material.opacity = 0.5f + (intensity - 0.625f) * 1.2f;
        float pulse = 0.1f * std::sin(elapsedTime * 8.0f);
        material.opacity = std::min(0.95f, material.opacity + pulse);
    }
}

int main() {


    Canvas canvas("plane with car");
    GLRenderer renderer(canvas.size());
    renderer.setClearColor(Color::white);

    auto scene = Scene::create();
    scene->background = Color::white;



    auto camera = PerspectiveCamera::create(75, canvas.aspect(), 0.1f, 1000);
    camera->position.set(0, 2, 5);


    // Load and rotate the deer, then add to container
    Car car;
    scene->add(car.getGroup());

    CollisionDetection collisionDetection(7.0f);


    AssimpLoader loader;


    //create mushrooms
    // Add blur state variables after canvas creation
    float blurLevel = 0.0f;
    const float maxBlur = 8.0f;
    const float blurDecayRate = 0.1f;
    const float blurPerMushroom = 1.0f;



    // Create blur overlay
    // Create blur overlay as orthographic overlay (not attached to camera)
    auto orthoCamera = OrthographicCamera::create(-1, 1, 1, -1, 0, 1);
    auto blurScene = Scene::create();

    auto blurGeometry = PlaneGeometry::create(2, 2);
    auto blurMaterial = MeshBasicMaterial::create();
    blurMaterial->color = Color(0.80f, 0.20f, 0.4f); // Pink
    blurMaterial->transparent = true;
    blurMaterial->opacity = 0.7f; // Fixed opacity for testing
    blurMaterial->depthTest = false;



    auto blurPlane = Mesh::create(blurGeometry, blurMaterial);
    blurScene->add(blurPlane);

    // Replace mushroom creation with GameObject
    // Load mushroom model ONCE
    auto mushroomTemplate = loader.load("../Models/mushrooms/model.obj");
    if (!mushroomTemplate) {
        std::cerr << "Failed to load mushroom model!" << std::endl;
        return -1;
    }

    std::vector<std::shared_ptr<GameObject>> mushrooms;
    for (int i = 0; i < 500; i++) {
        auto [x, z] = randomPosition();

        // Clone the template instead of loading from file
        auto mushroomClone = mushroomTemplate->children[0]->clone();

        mushroomClone->position.set(x, -5, z);
        mushroomClone->scale.set(20.0f, 20.0f, 20.0f);

        auto mushroom = std::make_shared<GameObject>(
            mushroomClone,  // Pass the cloned mesh
            15.0f,
            GameObjectType::MUSHROOM
        );

        mushroom->setOnCollision([&blurLevel](GameObject& obj) {
            blurLevel = std::min(blurLevel + 3.0f, 8.0f);
        });

        scene->add(mushroom->getModel());
        mushrooms.push_back(mushroom);
    }





    // Replace tree creation with GameObject
    std::vector<std::shared_ptr<GameObject>> trees;
    std::vector<Vector3> treePositions;

    for (int i = 0; i < 1000; i++) {
        auto [x, z] = randomPosition();
        auto tree = std::make_shared<GameObject>(
            "../Models/model.obj",
            threepp::Vector3(x, 65, z),
            threepp::Vector3(40.0f, 40.0f, 40.0f),
            7.0f,
            GameObjectType::TREE
        );

        scene->add(tree->getModel());
        trees.push_back(tree);
        treePositions.push_back(Vector3(x, 60, z));
    }

    // Set obstacles in collision detection
    collisionDetection.setObstacles(treePositions);

    // Create controller with collision detection
    KeyController controller(*car.getGroup(), collisionDetection);
    canvas.addKeyListener(controller);



    //light
    auto light = DirectionalLight::create();
    light->position.set(0,15,0);
    scene->add(light);

    auto ambient = AmbientLight::create(Color::white, 0.5f);
    scene->add(ambient);



    Turret turret("../Models/turretV1bla4.obj", 3.0f);
    turret.setRotation(0, 0, 0);
    turret.setPosition(0, -100.0f, 0);
    turret.setScale(0.03f, 0.03f, 0.03f);
    turret.attachTo(*car.getGroup(), threepp::Vector3(0, 1.0f, 0));
    canvas.addKeyListener(turret);

    std::vector<MovingBear> bears;
    try {
        spawnBears(*scene, bears, 500, "../Models/Mesh_Bear.obj", 0.1f); // removed stray ')'
    } catch (const std::exception& e) {
        std::cerr << "spawnBears failed: " << e.what() << std::endl;
        return -1;
    }


    OrbitControls controls{*camera, canvas};
    controls.target.set(0, 0, 0);    // orbit around the origin (or bil->position)
    controls.update();

    canvas.onWindowResize([&](WindowSize size) {
        camera->aspect = size.aspect();
        camera->updateProjectionMatrix();
        renderer.setSize(size);
    });

    Clock clock;
    canvas.animate([&]() {
        float dt = clock.getDelta();

        controller.update(dt);

        car.setSpeed(controller.getSpeed());
        car.setRotation(controller.getRotation());
        car.update(dt);

        // Check mushroom collisions
        threepp::Vector3 carPos = car.getGroup()->position;
        for (auto it = mushrooms.begin(); it != mushrooms.end();) {
            if ((*it)->checkCollision(carPos)) {
                (*it)->handleCollision();
                (*it)->removeFromScene(*scene);
                it = mushrooms.erase(it);
            } else {
                ++it;
            }
        }

        updateBlurEffect(blurLevel, *blurMaterial, dt, clock.getElapsedTime(), maxBlur, blurDecayRate);


        // Decay blur over time
        blurLevel = std::max(0.0f, blurLevel - blurDecayRate * dt);



        turret.update(dt);
        turret.updateBullets(dt, *scene);
        updateBears(bears, dt);
        updateCameraFollow(*camera, *car.getGroup());

        renderer.render(*scene, *camera);

        // Render blur overlay on top
        renderer.autoClear = false;
        renderer.render(*blurScene, *orthoCamera);
        renderer.autoClear = true;
    });
}