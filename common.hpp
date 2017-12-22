#ifndef COMMON_HPP
#define COMMON_HPP

#include <cmath>
#include <iostream>
#include <map>
#include <vector>

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/IO/File.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Urho2D/CollisionBox2D.h>
#include <Urho3D/Urho2D/CollisionCircle2D.h>
#include <Urho3D/Urho2D/ConstraintDistance2D.h>
#include <Urho3D/Urho2D/ConstraintFriction2D.h>
#include <Urho3D/Urho2D/ConstraintGear2D.h>
#include <Urho3D/Urho2D/ConstraintMotor2D.h>
#include <Urho3D/Urho2D/ConstraintMouse2D.h>
#include <Urho3D/Urho2D/ConstraintPrismatic2D.h>
#include <Urho3D/Urho2D/ConstraintPulley2D.h>
#include <Urho3D/Urho2D/ConstraintRevolute2D.h>
#include <Urho3D/Urho2D/ConstraintRope2D.h>
#include <Urho3D/Urho2D/ConstraintWeld2D.h>
#include <Urho3D/Urho2D/ConstraintWheel2D.h>
#include <Urho3D/Urho2D/Drawable2D.h>
#include <Urho3D/Urho2D/PhysicsEvents2D.h>
#include <Urho3D/Urho2D/PhysicsWorld2D.h>
#include <Urho3D/Urho2D/RigidBody2D.h>
#include <Urho3D/Urho2D/Sprite2D.h>
#include <Urho3D/Urho2D/StaticSprite2D.h>

using namespace Urho3D;

class Common : public Application {
    URHO3D_OBJECT(Common, Application);
public:
    Common(Context* context) : Application(context) {}
    virtual void Setup() override {
        this->engineParameters_[EP_FULL_SCREEN] = false;
        this->engineParameters_[EP_WINDOW_TITLE] = __FILE__;
        this->engineParameters_[EP_WINDOW_HEIGHT] = 512;
        this->engineParameters_[EP_WINDOW_WIDTH] = 512;
    }
    virtual void Start() override {
        if (!this->scene) {
            this->input = this->GetSubsystem<Input>();
            this->input->SetMouseVisible(true);
            this->resourceCache = GetSubsystem<ResourceCache>();
            SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(Common, HandlePostRenderUpdate));
            SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Common, HandleUpdate));
            SubscribeToEvent(E_PHYSICSBEGINCONTACT2D, URHO3D_HANDLER(Common, HandlePhysicsBeginContact2D));
            SubscribeToEvent(E_PHYSICSPRESTEP, URHO3D_HANDLER(Common, HandlePhysicsPreStep));
            SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(Common, HandleMouseButtonDown));
            StartOnce();
        }

        // Scene
        this->scene = new Scene(this->context_);
        this->scene->CreateComponent<Octree>();
        this->scene->CreateComponent<DebugRenderer>();
        this->physicsWorld = this->scene->CreateComponent<PhysicsWorld2D>();
        this->physicsWorld->SetGravity(Vector2(0.0f, 0.0f));
        this->physicsWorld->SetDrawAabb(false);
        this->physicsWorld->SetDrawCenterOfMass(true);
        this->physicsWorld->SetDrawJoint(true);
        this->physicsWorld->SetDrawPair(true);
        this->physicsWorld->SetDrawShape(true);

        // Camera
        this->cameraNode = this->scene->CreateChild("Camera");
        this->cameraNode->SetPosition(Vector3(this->windowWidth / 2.0f, this->windowHeight / 2.0f, -1.0f));
        this->camera = this->cameraNode->CreateComponent<Camera>();
        this->camera->SetOrthoSize(this->windowWidth);
        this->camera->SetOrthographic(true);
        auto renderer = GetSubsystem<Renderer>();
        SharedPtr<Viewport> viewport(new Viewport(this->context_, this->scene, this->cameraNode->GetComponent<Camera>()));
        renderer->SetViewport(0, viewport);

        auto dummyNode = this->scene->CreateChild("Dummy");
        this->dummyBody = dummyNode->CreateComponent<RigidBody2D>();
        this->pickedNode = nullptr;

        // Non-urho.
        this->steps = 0;
        this->debugEvents = false;

        this->StartExtra();
    }
    virtual void Stop() override {}
protected:
    bool debugEvents;
    Camera *camera;
    Input *input;
    Node *cameraNode, *dummyNode, *pickedNode;
    PhysicsWorld2D *physicsWorld;
    ResourceCache *resourceCache;
    RigidBody2D *dummyBody;
    SharedPtr<Scene> scene;
    uint64_t steps;
    // Generate robot input
    // TODO use a spacial index instead.
    std::vector<std::pair<Vector2, Node *>> worldVoxel;

    /// Everything in the scene should be proportional to this number,
    /// so that we can set it to anything we want without changing anything.
    ///
    /// Our perfect symmetry is broken however by evil things like Box2D thresholds
    /// as explained in velocity_stop.cpp, going close to 1.0f is a bad idea.
    static constexpr float windowWidth = 10.0f;
    static constexpr float windowHeight = windowWidth;
    static constexpr float cameraSpeed = 1.0;
    static constexpr float cameraZoomSpeed = 0.5f;
    static constexpr unsigned int voxelResolution = 100;

    Vector2 GetMousePositionWorld() {
        auto graphics = GetSubsystem<Graphics>();
        auto screenPoint = Vector3(
            (float)this->input->GetMousePosition().x_ / graphics->GetWidth(),
            (float)this->input->GetMousePosition().y_ / graphics->GetHeight(),
            0.0f
        );
        auto worldPoint = this->camera->ScreenToWorldPoint(screenPoint);
        return Vector2(worldPoint.x_, worldPoint.y_);
    }

    void HandlePhysicsBeginContact2D(StringHash eventType, VariantMap& eventData) {
        this->HandlePhysicsBeginContact2DExtra(eventType, eventData);
    }

    void HandleMouseButtonDown(StringHash eventType, VariantMap& eventData) {
        auto mousePositionWorld = this->GetMousePositionWorld();
        auto rigidBody = this->physicsWorld->GetRigidBody(mousePositionWorld);
        if (rigidBody) {
            this->pickedNode = rigidBody->GetNode();
            auto constraintMouse = this->pickedNode->CreateComponent<ConstraintMouse2D>();
            constraintMouse->SetTarget(mousePositionWorld);
            constraintMouse->SetMaxForce(1000 * rigidBody->GetMass());
            constraintMouse->SetCollideConnected(true);
            constraintMouse->SetOtherBody(this->dummyBody);
        }
        SubscribeToEvent(E_MOUSEMOVE, URHO3D_HANDLER(Common, HandleMouseMove));
        SubscribeToEvent(E_MOUSEBUTTONUP, URHO3D_HANDLER(Common, HandleMouseButtonUp));
    }

    virtual void HandlePhysicsBeginContact2DExtra(StringHash eventType, VariantMap& eventData) {}

    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData) {
        this->physicsWorld->DrawDebugGeometry();
    }

    void HandleMouseButtonUp(StringHash eventType, VariantMap& eventData) {
        if (this->pickedNode) {
            this->pickedNode->RemoveComponent<ConstraintMouse2D>();
            this->pickedNode = nullptr;
        }
        UnsubscribeFromEvent(E_MOUSEMOVE);
        UnsubscribeFromEvent(E_MOUSEBUTTONUP);
    }

    void HandleMouseMove(StringHash eventType, VariantMap& eventData) {
        if (this->pickedNode) {
            auto constraintMouse = this->pickedNode->GetComponent<ConstraintMouse2D>();
            constraintMouse->SetTarget(this->GetMousePositionWorld());
        }
    }

    void HandleUpdate(StringHash eventType, VariantMap& eventData) {
        using namespace Update;
        auto timeStep = eventData[P_TIMESTEP].GetFloat();
        if (this->debugEvents) {
            std::cout << "HandleUpdate" << std::endl;
            std::cout << "timeStep = " << timeStep << std::endl;
        }
        auto cameraStep = this->camera->GetOrthoSize() * (1.0f / this->camera->GetZoom()) * this->cameraSpeed * timeStep;

        // Camera
        if (this->input->GetKeyDown(KEY_DOWN)) {
            this->cameraNode->Translate(Vector2::DOWN * cameraStep);
        }
        if (this->input->GetKeyDown(KEY_LEFT)) {
            this->cameraNode->Translate(Vector2::LEFT * cameraStep);
        }
        if (input->GetKeyDown(KEY_PAGEUP)) {
            this->camera->SetZoom(this->camera->GetZoom() * 1.0f / std::pow(this->cameraZoomSpeed, timeStep));
        }
        if (input->GetKeyDown(KEY_PAGEDOWN)) {
            this->camera->SetZoom(this->camera->GetZoom() * std::pow(this->cameraZoomSpeed, timeStep));
        }
        if (this->input->GetKeyDown(KEY_RIGHT)) {
            this->cameraNode->Translate(Vector2::RIGHT * cameraStep);
        }
        if (this->input->GetKeyDown(KEY_UP)) {
            this->cameraNode->Translate(Vector2::UP * cameraStep);
        }

        // Scene state
        if (this->input->GetKeyDown(KEY_ESCAPE)) {
            engine_->Exit();
        }
        if (this->input->GetKeyDown(KEY_F5)) {
            File saveFile(this->context_, GetSubsystem<FileSystem>()->GetProgramDir() + String(this->steps) + ".xml", FILE_WRITE);
            this->scene->SaveXML(saveFile);
        }
        if (this->input->GetKeyDown(KEY_R)) {
            this->scene->Clear();
            this->Start();
        }

        if (false) {
            this->worldVoxel.clear();
            for (unsigned int x = 0; x < this->voxelResolution; ++x) {
                for (unsigned int y = 0; y < this->voxelResolution; ++y) {
                    auto worldPosition = Vector2(
                        x * this->windowHeight / this->voxelResolution,
                        y * this->windowWidth / this->voxelResolution
                    );
                    auto rigidBody = this->physicsWorld->GetRigidBody(Vector2(worldPosition));
                    if (rigidBody) {
                        auto node = rigidBody->GetNode();
                        this->worldVoxel.push_back(std::make_pair(worldPosition, node));
                        //std::cout << x << " " << y << " " << node->GetName().CString() << std::endl;
                    }
                }
            }
        }
        this->HandleUpdateExtra(eventType, eventData);
        this->steps++;
    }

    void HandlePhysicsPreStep(StringHash eventType, VariantMap& eventData) {
        using namespace PhysicsPreStep;
        auto timeStep = eventData[P_TIMESTEP].GetFloat();
        if (this->debugEvents) {
            std::cout << "PhysicsPreStep" << std::endl;
            std::cout << "timeStep = " << timeStep << std::endl;
        }
    }

    virtual void HandleUpdateExtra(StringHash eventType, VariantMap& eventData) {}

    virtual void StartExtra() {}

    /// Start steps that are not rerun when restart the scene,
    /// only the first time the application starts.
    virtual void StartOnce() {}
};

#endif
