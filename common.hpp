#ifndef COMMON_HPP
#define COMMON_HPP

#include <cassert>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <unordered_map>
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
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/IO/File.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Math/StringHash.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/LogicComponent.h>
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
        this->engineParameters_[EP_WINDOW_HEIGHT] = 950;
        this->engineParameters_[EP_WINDOW_WIDTH] = 950;
    }
    virtual void Start() override {
        if (!this->scene) {
            this->input = this->GetSubsystem<Input>();
            this->input->SetMouseVisible(true);
            this->resourceCache = this->GetSubsystem<ResourceCache>();
            this->resourceCache->AddResourceDir(GetParentPath(__FILE__));
            this->font = this->resourceCache->GetResource<Font>("Fonts/Anonymous Pro.ttf");
            this->ui = this->GetSubsystem<UI>();
            this->SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(Common, HandleKeyDown));
            this->SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(Common, HandleMouseButtonDown));
            this->SubscribeToEvent(E_PHYSICSBEGINCONTACT2D, URHO3D_HANDLER(Common, HandlePhysicsBeginContact2D));
            this->SubscribeToEvent(E_PHYSICSPRESTEP, URHO3D_HANDLER(Common, HandlePhysicsPreStep));
            this->SubscribeToEvent(E_PHYSICSUPDATECONTACT2D, URHO3D_HANDLER(Common, HandlePhysicsUpdateContact2D));
            this->SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(Common, HandlePostRenderUpdate));
            this->SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Common, HandleUpdate));
            this->StartOnce();
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
        this->cameraNode->SetPosition(Vector3(this->GetWindowWidth() / 2.0f, this->GetWindowHeight() / 2.0f, -1.0f));
        this->CreateCamera(this->cameraNode, this->GetWindowWidth());

        // Mouse drag
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
    static constexpr float cameraSpeed = 1.0;
    static constexpr float cameraZoomSpeed = 0.5f;
    static constexpr unsigned int voxelResolution = 100;

    bool debugEvents;
    bool drawDebugGeometry;
    Camera *camera;
    Font *font;
    Input *input;
    Node *cameraNode, *dummyNode, *pickedNode;
    PhysicsWorld2D *physicsWorld;
    ResourceCache *resourceCache;
    RigidBody2D *dummyBody;
    SharedPtr<Scene> scene;
    SharedPtr<Viewport> viewport;
    UI *ui;
    uint64_t steps;
    // Generate robot input
    // TODO use a spacial index instead.
    std::vector<std::pair<Vector2, Node *>> worldVoxel;

    void CreateCamera(Node *node, float orthoSize) {
        this->camera = node->CreateComponent<Camera>();
        this->camera->SetOrthoSize(orthoSize);
        this->camera->SetOrthographic(true);
        this->viewport = SharedPtr<Viewport>(new Viewport(this->context_, this->scene, this->camera));
        GetSubsystem<Renderer>()->SetViewport(0, this->viewport);
    }

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

    virtual float GetWindowHeight() const { return this->GetWindowWidth(); }
    /**
     * Everything in the scene should be proportional to this number,
     * so that we can set it to anything we want without changing anything.
     *
     * Our perfect symmetry is broken however by evil things like Box2D thresholds
     * as explained in velocity_stop.cpp, going close to 1.0f is a bad idea.
     */
    virtual float GetWindowWidth() const { return 10.0f; }

    void HandlePhysicsBeginContact2D(StringHash eventType, VariantMap& eventData) {
        this->HandlePhysicsBeginContact2DExtra(eventType, eventData);
    }

    virtual void HandlePhysicsBeginContact2DExtra(StringHash eventType, VariantMap& eventData) {}

    void HandleKeyDown(StringHash eventType, VariantMap& eventData) {
        using namespace KeyDown;
        auto key = eventData[P_KEY].GetInt();
        if (key == KEY_ESCAPE) {
            this->engine_->Exit();
        } else if (key == KEY_F5) {
            this->Reset();
        } else if (key == KEY_F6) {
            File saveFile(this->context_, GetSubsystem<FileSystem>()->GetProgramDir() + String(this->steps) + ".xml", FILE_WRITE);
            this->scene->SaveXML(saveFile);
        }
        this->HandleKeyDownExtra(eventType, eventData);
    }

    virtual void HandleKeyDownExtra(StringHash /*eventType*/, VariantMap& /*eventData*/) {}

    void HandlePhysicsUpdateContact2D(StringHash eventType, VariantMap& eventData) {
        this->HandlePhysicsUpdateContact2DExtra(eventType, eventData);
    }

    virtual void HandlePhysicsUpdateContact2DExtra(StringHash eventType, VariantMap& eventData) {}

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
        this->SubscribeToEvent(E_MOUSEMOVE, URHO3D_HANDLER(Common, HandleMouseMove));
        this->SubscribeToEvent(E_MOUSEBUTTONUP, URHO3D_HANDLER(Common, HandleMouseButtonUp));
    }

    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData) {
        if (this->drawDebugGeometry) {
            this->physicsWorld->DrawDebugGeometry();
        }
    }

    void HandleMouseButtonUp(StringHash eventType, VariantMap& eventData) {
        if (this->pickedNode) {
            this->pickedNode->RemoveComponent<ConstraintMouse2D>();
            this->pickedNode = nullptr;
        }
        this->UnsubscribeFromEvent(E_MOUSEMOVE);
        this->UnsubscribeFromEvent(E_MOUSEBUTTONUP);
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
            this->cameraNode->Translate2D(Vector2::DOWN * cameraStep);
        }
        if (this->input->GetKeyDown(KEY_LEFT)) {
            this->cameraNode->Translate2D(Vector2::LEFT * cameraStep);
        }
        if (input->GetKeyDown(KEY_PAGEUP)) {
            this->camera->SetZoom(this->camera->GetZoom() * 1.0f / std::pow(this->cameraZoomSpeed, timeStep));
        }
        if (input->GetKeyDown(KEY_PAGEDOWN)) {
            this->camera->SetZoom(this->camera->GetZoom() * std::pow(this->cameraZoomSpeed, timeStep));
        }
        if (this->input->GetKeyDown(KEY_RIGHT)) {
            this->cameraNode->Translate2D(Vector2::RIGHT * cameraStep);
        }
        if (this->input->GetKeyDown(KEY_UP)) {
            this->cameraNode->Translate2D(Vector2::UP * cameraStep);
        }

        // Generate robot voxel input.
        if (false) {
            this->worldVoxel.clear();
            for (unsigned int x = 0; x < this->voxelResolution; ++x) {
                for (unsigned int y = 0; y < this->voxelResolution; ++y) {
                    auto worldPosition = Vector2(
                        // TODO override this->GetWindowWidth() and use that here.
                        x * this->GetWindowHeight() / this->voxelResolution,
                        y * this->GetWindowWidth() / this->voxelResolution
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

    void Reset() {
        this->scene->Clear();
        this->ui->Clear();
        this->Start();
    }

    virtual void StartExtra() {}

    /// Start steps that are not rerun when restart the scene,
    /// only the first time the application starts.
    virtual void StartOnce() {}
};

/// Move up to a given maximum distance from spawn point, then turn around.
class MaxDistComponent : public LogicComponent {
public:
    MaxDistComponent(Context* context) : LogicComponent(context) {}
    virtual void Start() override {
        this->Reset();
        this->initialPosition = this->node_->GetPosition2D();
        this->maxBouncesGiven = false;
        this->previousPosition = this->initialPosition;
    }
    virtual void Update(float timeStep) override {
        if (this->active) {
            this->node_->Translate2D(this->speed * timeStep);
            if ((this->node_->GetPosition2D() - this->initialPosition).Length() > this->maxDist) {
                this->speed *= -1.0f;
                this->bounces++;
            } else if (
                this->maxBouncesGiven &&
                this->bounces == this->maxBounces &&
                (
                    (this->initialPosition - this->node_->GetPosition2D()).DotProduct(
                    this->initialPosition - this->previousPosition) < 0.0f
                )
            ) {
                this->active = false;
            } else {
                this->previousPosition = this->node_->GetPosition2D();
            }
        }
    }
    void Reset() {
        this->bounces = 0;
        this->active = true;
    }
    void SetActive(bool active) { this->active = active; }
    void SetSpeed(Vector2 speed) { this->speed = speed; }
    void SetMaxDist(float maxDist) { this->maxDist = maxDist; }
    void SetMaxBounces(unsigned int maxBounces) {
        this->maxBounces = maxBounces;
        this->maxBouncesGiven = true;
    }
private:
    Vector2 initialPosition, previousPosition, speed, initialSpeed;
    float maxDist;
    unsigned int bounces, maxBounces;
    bool active, maxBouncesGiven;
};

#endif
