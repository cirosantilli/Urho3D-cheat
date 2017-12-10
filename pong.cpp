/*
Single player pong-like game, more like squash.

TODO:

- the player does not fully touch the ground, there is a gap.
- make player edges rounded. Probably attach two colision circles to the edgest of the player.
- ball gets too fast when player hits it while moving up / down. How to prevent this?
- ball speed can get too vertical, and it takes forever to hit wall and come back
- ball speed can get too horizontal, and then it becomes too trivial. We need some random / adversarial aspect to make it more interesting.
*/

#include <cmath>
#include <iostream>

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/Object.h>
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
#include <Urho3D/Urho2D/PhysicsEvents2D.h>
#include <Urho3D/Urho2D/PhysicsWorld2D.h>
#include <Urho3D/Urho2D/RigidBody2D.h>

using namespace Urho3D;

class Main : public Application {
    URHO3D_OBJECT(Main, Application);
public:
    Main(Context* context) : Application(context) {}
    virtual void Setup() override {
        engineParameters_[EP_FULL_SCREEN] = false;
        engineParameters_[EP_WINDOW_TITLE] = __FILE__;
        engineParameters_[EP_WINDOW_HEIGHT] = 512;
        engineParameters_[EP_WINDOW_WIDTH] = 512;
    }
    void Start() override {
        if (this->scene) {
            this->scene->Clear();
        } else {
            this->scene = new Scene(this->context_);
            this->input = this->GetSubsystem<Input>();

            // Events
            SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(Main, HandleKeyDown));
            SubscribeToEvent(E_PHYSICSBEGINCONTACT2D, URHO3D_HANDLER(Main, HandlePhysicsBeginContact2D));
            SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(Main, HandlePostRenderUpdate));
            SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Main, HandleUpdate));

            // Score
            this->text = this->GetSubsystem<UI>()->GetRoot()->CreateChild<Text>();
            this->text->SetFont(this->GetSubsystem<ResourceCache>()->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);
            this->text->SetHorizontalAlignment(HA_CENTER);
            this->text->SetVerticalAlignment(VA_CENTER);
        }

        // Application state.
        this->score = 0;
        this->text->SetText(std::to_string(this->score).c_str());
        this->steps = 0;

        // Scene
        {
            this->scene->CreateComponent<Octree>();
            this->scene->CreateComponent<DebugRenderer>();
            this->scene->CreateComponent<PhysicsWorld2D>();
            this->physicsWorld = this->scene->GetComponent<PhysicsWorld2D>();
            this->physicsWorld->SetGravity(Vector2(0.0f, 0.0f));

            // Graphics
            this->cameraNode = this->scene->CreateChild("Camera");
            this->cameraNode->SetPosition(Vector3(windowWidth / 2.0f, windowHeight / 2.0f, -1.0f));
            this->camera = this->cameraNode->CreateComponent<Camera>();
            this->camera->SetOrthographic(true);
            this->camera->SetOrthoSize(windowWidth);
            auto renderer = GetSubsystem<Renderer>();
            SharedPtr<Viewport> viewport(new Viewport(context_, this->scene, this->cameraNode->GetComponent<Camera>()));
            renderer->SetViewport(0, viewport);

            Node *bottomWallNode;
            RigidBody2D *wallBody;
            {
                bottomWallNode = this->scene->CreateChild("BottomWall");
                bottomWallNode->SetPosition(Vector3(this->windowWidth / 2.0, wallWidth / 2.0f, 0.0f));
                wallBody = bottomWallNode->CreateComponent<RigidBody2D>();
                auto box = bottomWallNode->CreateComponent<CollisionBox2D>();
                box->SetSize(Vector2(wallLength, wallWidth));
                box->SetRestitution(0.0);
            } {
                auto node = bottomWallNode->Clone();
                node->SetName("TopWall");
                node->SetPosition(Vector3(this->windowWidth / 2.0f, windowHeight - (wallWidth / 2.0f), 0.0f));
            } {
                auto& node = this->rightWallNode;
                node = bottomWallNode->Clone();
                node->SetName("RightWall");
                node->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
                node->SetPosition(Vector3(this->windowWidth - (wallWidth / 2.0f), windowHeight / 2.0f, 0.0f));
            } {
                auto& node = this->leftWallNode;
                node = bottomWallNode->Clone();
                node->SetName("LeftWall");
                node->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
                node->SetPosition(Vector3(-wallWidth / 2.0f, windowHeight / 2.0f, 0.0f));
            } {
                auto& node = this->playerNode;
                node = bottomWallNode->Clone();
                node->SetName("Player");
                node->SetRotation(Quaternion(0.0f, 0.0f, 90.0f));
                // TODO The more elegant value of x is wallWidth / 2.0. But then we get stuck
                // to the left wall when the ball hits the player. Use collision filtering.
                node->SetPosition(Vector3(wallWidth, windowHeight / 2.0f, 0.0f));

                auto body = node->GetComponent<RigidBody2D>();
                body->SetBodyType(BT_DYNAMIC);
                body->SetFixedRotation(true);
                body->SetLinearDamping(4.0);

                auto constraint = node->CreateComponent<ConstraintPrismatic2D>();
                constraint->SetOtherBody(wallBody);
                constraint->SetAxis(Vector2(0.0f, 1.0f));
                constraint->SetAnchor(Vector2(this->windowWidth / 2.0f, 0.0f));
                constraint->SetCollideConnected(true);

                //ConstraintPrismatic2D* constraintPrismatic = boxPrismaticNode->CreateComponent<ConstraintPrismatic2D>();
                //constraintPrismatic->SetOtherBody(ballPrismaticNode->GetComponent<RigidBody2D>()); // Constrain ball to box
                //constraintPrismatic->SetAxis(Vector2(1.0f, 1.0f)); // Slide from [0,0] to [1,1]
                //constraintPrismatic->SetAnchor(Vector2(4.0f, 2.0f));
                //constraintPrismatic->SetLowerTranslation(-1.0f);
                //constraintPrismatic->SetUpperTranslation(0.5f);
                //constraintPrismatic->SetEnableLimit(true);
                //constraintPrismatic->SetMaxMotorForce(1.0f);
                //constraintPrismatic->SetMotorSpeed(0.0f);

                auto shape = node->GetComponent<CollisionBox2D>();
                shape->SetDensity(this->playerDensity);
                shape->SetFriction(1.0f);
                shape->SetRestitution(0.0);
                shape->SetSize(Vector2(playerLength, wallWidth));
            } {
                this->ballNode = this->scene->CreateChild("Ball");
                this->ballNode->SetPosition(Vector3(this->windowWidth / 4.0f, windowHeight / 2.0f, 0.0f));
                auto body = this->ballNode->CreateComponent<RigidBody2D>();
                body->SetBodyType(BT_DYNAMIC);
                body->SetLinearVelocity(Vector2(2 * this->windowWidth, -windowHeight / 2.0f));
                auto shape = this->ballNode->CreateComponent<CollisionCircle2D>();
                shape->SetDensity(1.0f);
                shape->SetFriction(0.0f);
                shape->SetRadius(ballRadius);
                shape->SetRestitution(ballRestitution);
            }
        }
    }
    void Stop() override {}
private:
    Camera *camera;
    Input *input;
    Node *ballNode, *cameraNode, *leftWallNode, *playerNode, *rightWallNode;
    PhysicsWorld2D *physicsWorld;
    SharedPtr<Scene> scene;
    Text *text;
    uint64_t score, steps;

    static constexpr float playerDensity = 10.0f;
    static constexpr float windowWidth = 1.0f;
    static constexpr float windowHeight = windowWidth;
    static constexpr float wallLength = windowWidth;
    static constexpr float wallWidth = windowWidth / 20.0f;
    static constexpr float ballRadius = windowWidth / 20.0f;
    static constexpr float ballRestitution = 1.0f;
    static constexpr float playerLength = windowHeight / 4.0f;
    static constexpr float cameraSpeed = windowHeight;
    static constexpr float cameraZoomSpeed = 0.5f;

    void HandleKeyDown(StringHash /*eventType*/, VariantMap& eventData) {
        using namespace KeyDown;
        int key = eventData[P_KEY].GetInt();
        if (key == KEY_ESCAPE) {
            engine_->Exit();
        } else if (key == KEY_R) {
            this->Start();
        }
    }

    void HandlePhysicsBeginContact2D(StringHash eventType, VariantMap& eventData) {
        using namespace PhysicsBeginContact2D;
        auto nodea = static_cast<Node*>(eventData[P_NODEA].GetPtr());
        auto nodeb = static_cast<Node*>(eventData[P_NODEB].GetPtr());
        Node *otherNode;
        bool isBall = false;
        if (nodea == this->ballNode) {
            otherNode = nodeb;
            isBall = true;
        }
        if (nodeb == this->ballNode) {
            otherNode = nodea;
            isBall = true;
        }
        if (otherNode == this->rightWallNode) {
            this->score++;
        } else if (otherNode == this->leftWallNode) {
            this->score = 0;
        }
        this->text->SetText(std::to_string(this->score).c_str());
    }

    void HandleUpdate(StringHash eventType, VariantMap& eventData) {
        using namespace Update;
        auto timeStep = eventData[P_TIMESTEP].GetFloat();
        auto impuseMagnitude = this->windowWidth * this->playerDensity * timeStep * 10.0;
        auto body = this->playerNode->GetComponent<RigidBody2D>();
        if (this->input->GetKeyDown(KEY_DOWN)) {
            body->ApplyForceToCenter(Vector2::DOWN * impuseMagnitude, true);
        }
        if (this->input->GetKeyDown(KEY_UP)) {
            body->ApplyForceToCenter(Vector2::UP * impuseMagnitude, true);
        }
        if (this->input->GetKeyDown(KEY_RIGHT)) {
            body->ApplyForceToCenter(Vector2::RIGHT * impuseMagnitude, true);
        }
        if (this->input->GetKeyDown(KEY_LEFT)) {
            body->ApplyForceToCenter(Vector2::LEFT * impuseMagnitude, true);
        }
        if (this->input->GetKeyDown(KEY_S)) {
            this->cameraNode->Translate(Vector2::DOWN * this->cameraSpeed * timeStep);
        }
        if (this->input->GetKeyDown(KEY_W)) {
            this->cameraNode->Translate(Vector2::UP * this->cameraSpeed * timeStep);
        }
        if (this->input->GetKeyDown(KEY_D)) {
            this->cameraNode->Translate(Vector2::RIGHT * this->cameraSpeed * timeStep);
        }
        if (this->input->GetKeyDown(KEY_A)) {
            this->cameraNode->Translate(Vector2::LEFT * this->cameraSpeed * timeStep);
        }
        if (input->GetKeyDown(KEY_PAGEUP)) {
            this->camera->SetZoom(this->camera->GetZoom() * 1.0f / std::pow(this->cameraZoomSpeed, timeStep));
        }
        if (input->GetKeyDown(KEY_PAGEDOWN)) {
            this->camera->SetZoom(this->camera->GetZoom() * std::pow(this->cameraZoomSpeed, timeStep));
        }
        this->steps++;
    }

    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData) {
        this->physicsWorld->DrawDebugGeometry();
    }

};

URHO3D_DEFINE_APPLICATION_MAIN(Main);
