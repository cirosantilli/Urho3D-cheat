#ifndef COMMON_HPP
#define COMMON_HPP

/*
Expected outcome:

- press ESC: quit
- release space: stdout shows a message
*/

#include <cmath>
#include <iostream>

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
#include <Urho3D/Urho2D/ConstraintPrismatic2D.h>
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
        this->cameraNode->SetPosition(Vector3(windowWidth / 2.0f, windowHeight / 2.0f, -1.0f));
        this->camera = this->cameraNode->CreateComponent<Camera>();
        this->camera->SetOrthoSize(windowWidth);
        this->camera->SetOrthographic(true);
        auto renderer = GetSubsystem<Renderer>();
        SharedPtr<Viewport> viewport(new Viewport(context_, this->scene, this->cameraNode->GetComponent<Camera>()));
        renderer->SetViewport(0, viewport);

        // Non-urho.
        this->steps = 0;

        this->StartExtra();
    }
    virtual void Stop() override {}
protected:
    Camera *camera;
    Input *input;
    Node *cameraNode;
    PhysicsWorld2D *physicsWorld;
    ResourceCache *resourceCache;
    SharedPtr<Scene> scene;
    uint64_t steps;

    /// Everything in the scene should be proportional to this number,
    /// so that we can set it to anything we want without changing anything.
    ///
    /// Our perfect symmetry is broken however by evil things like Box2D thresholds
    /// as explained in velocity_stop.cpp, going close to 1.0f is a bad idea.
    static constexpr float windowWidth = 10.0f;
    static constexpr float windowHeight = windowWidth;
    static constexpr float cameraSpeed = 1.0;
    static constexpr float cameraZoomSpeed = 0.5f;

    void HandlePhysicsBeginContact2D(StringHash eventType, VariantMap& eventData) {
        this->HandlePhysicsBeginContact2DExtra(eventType, eventData);
    }
    virtual void HandlePhysicsBeginContact2DExtra(StringHash eventType, VariantMap& eventData) {}
    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData) {
        this->physicsWorld->DrawDebugGeometry();
    }
    void HandleUpdate(StringHash eventType, VariantMap& eventData) {
        using namespace Update;
        auto timeStep = eventData[P_TIMESTEP].GetFloat();
        auto cameraStep = this->camera->GetOrthoSize() * (1.0f / this->camera->GetZoom()) * this->cameraSpeed * timeStep;
        if (this->input->GetKeyDown(KEY_DOWN)) {
            this->cameraNode->Translate(Vector2::DOWN * cameraStep);
        }
        if (this->input->GetKeyDown(KEY_ESCAPE)) {
            engine_->Exit();
        }
        if (this->input->GetKeyDown(KEY_F5)) {
            File saveFile(this->context_, GetSubsystem<FileSystem>()->GetProgramDir() + String(this->steps) + ".xml", FILE_WRITE);
            this->scene->SaveXML(saveFile);
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
        if (this->input->GetKeyDown(KEY_R)) {
            this->scene->Clear();
            this->Start();
        }
        if (this->input->GetKeyDown(KEY_RIGHT)) {
            this->cameraNode->Translate(Vector2::RIGHT * cameraStep);
        }
        if (this->input->GetKeyDown(KEY_UP)) {
            this->cameraNode->Translate(Vector2::UP * cameraStep);
        }
        this->HandleUpdateExtra(eventType, eventData);
        this->steps++;
    }
    virtual void HandleUpdateExtra(StringHash eventType, VariantMap& eventData) {}
    virtual void StartExtra() {}
    /// Start steps that are not rerun when restart the scene,
    /// only the first time the application starts.
    virtual void StartOnce() {}
};

#endif
