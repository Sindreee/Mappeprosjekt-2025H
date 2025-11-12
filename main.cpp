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
using namespace threepp;



void updateCameraFollow(PerspectiveCamera& camera, Object3D& target, float offsetDistance = 15.0f, float offsetHeight = 3.0f) {
    float dirX = std::sin(target.rotation.y);
    float dirZ = std::cos(target.rotation.y);

    camera.position.x = target.position.x - dirX * offsetDistance;
    camera.position.y = target.position.y + offsetHeight;
    camera.position.z = target.position.z - dirZ * offsetDistance;

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




int main() {


    Canvas canvas("plane with car");
    GLRenderer renderer(canvas.size());
    renderer.setClearColor(Color::white);

    auto scene = Scene::create();
    scene->background = Color::white;



    auto camera = PerspectiveCamera::create(75, canvas.aspect(), 0.1f, 1000);
    camera->position.set(0, 2, 5);

    //setRotationFromAxisAngle(Vector3{0,1,0}, angle)

    //load the car model
    // Create a container for the deer




    // Load and rotate the deer, then add to container
    Car car;
    scene->add(car.getGroup());

    CollisionDetection collisionDetection(7.0f);


    AssimpLoader loader;


    //create mushroomslkhsaduytrewytrewszdfhuiheqadgtfvhujniæåp879t46   |q1az>V

    auto mushroom = loader.load("../Models/mushrooms/model.obj");
    if (!mushroom) {
        std::cerr << "Failed to load model" << std::endl;
        return -1;
    }



    for (int i = 0; i < 500; i++) {
        auto mushroomclone = (i == 0) ? mushroom : mushroom->clone();
        auto [x,z] = randomPosition();
        mushroomclone->position.set(x,0,z);
        mushroom->scale.set(20.0f, 20.0f, 20.0f);
        scene->add(mushroomclone);
    }





    auto tree = loader.load("../Models/model.obj");
    if (!tree) {
        std::cerr << "Failed to load model" << std::endl;
        return -1;
    }

    std::vector<Vector3> treePositions;
    for (int i = 0; i < 1000; i++) {
        auto treeClone = (i == 0) ? tree : tree->clone();
        auto [x, z] = randomPosition();
        treeClone->position.set(x, 65, z);
        treeClone->scale.set(40.0f, 40.0f, 40.0f);
        scene->add(treeClone);
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


    std::vector<MovingBear> bears;
    try {
        spawnBears(*scene, bears, 400, "../Models/Mesh_Bear.obj", 0.1f); // removed stray ')'
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

        updateBears(bears, dt);

        updateCameraFollow(*camera, *car.getGroup());  // Dereference the shared_ptr

        renderer.render(*scene, *camera);
});
}