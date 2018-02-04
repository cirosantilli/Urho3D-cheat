/*
One agent eats apples and gets happy.
*/

#include "common.hpp"

using namespace Urho3D;

static const StringHash RESPAWN("Respawn");

class Main : public Common {
public:
    Main(Context* context) : Common(context) {
        this->sceneIdx = 0;
        context->RegisterFactory<ActivateDoorButtonComponent>();
        context->RegisterFactory<AppleButtonsAndComponent>();
        context->RegisterFactory<MaxDistComponent>();
    }
    virtual void StartExtra() override {

        // Application state.
        this->text = this->ui->GetRoot()->CreateChild<Text>();
        this->text->SetFont(this->font, 20);
        this->text->SetAlignment(HA_RIGHT, VA_BOTTOM);
        this->text->SetPosition(-10, -10);
        this->SetScore(0.0f);
        this->windowWidth = 20.0f * Main::playerRadius;

        // Scene
        {
            std::unordered_map<size_t,std::function<void()>> {
                {Main::sceneNameToIdx.at("tutorial"), [&](){
                    this->SetTitle("Tutorial");
                    this->CreateWallNodes();
                    this->CreatePlayerNode(Vector2(this->GetWindowWidth() / 4.0f, this->GetWindowHeight() / 2.0f));
                    auto text = this->ui->GetRoot()->CreateChild<Text>();
                    text->SetFont(this->font, 20);
                    text->SetAlignment(HA_CENTER, VA_CENTER);
                    text->SetText(
                        "W: forward\n"
                        "Q: turn left\n"
                        "E: turn right\n"
                        "N: next scene\n"
                        "P: previous scene\n"
                        "ESC: quit\n"
                    );
                }},
                {Main::sceneNameToIdx.at("apple"), [&](){
                    this->SetTitle("Apples are good");
                    this->CreateWallNodes();
                    this->CreateRandomPlayerNode();
                    this->CreateRandomAppleNode();
                }},
                {Main::sceneNameToIdx.at("golden-apple"), [&](){
                    this->SetTitle("Golden apples are better");
                    this->CreateWallNodes();
                    this->CreateRandomPlayerNode();
                    this->CreateRandomGoldenAppleNode();
                }},
                {Main::sceneNameToIdx.at("rotten-apple"), [&](){
                    this->SetTitle("Rotten apples are bad");
                    this->CreateWallNodes();
                    this->CreateRandomPlayerNode();
                    this->CreateRandomAppleNode();
                    this->CreateRandomRottenAppleNode();
                }},
                {Main::sceneNameToIdx.at("hole-top-bottom"), [&](){
                    this->SetTitle("I don't see no apple");
                    this->CreateWallNodes();
                    this->CreatePlayerNode(Vector2(this->GetWindowWidth() / 4.0f, this->GetWindowHeight() / 2.0f), -90.0f);
                    this->CreateAppleNode(Vector2(3.0f * this->GetWindowWidth() / 4.0f, this->GetWindowHeight() / 2.0f));
                    {
                        auto node = this->scene->CreateChild("SeparatorWall");
                        node->SetPosition2D(Vector2(this->GetWindowWidth() / 2.0, this->GetWindowHeight() / 2.0f));
                        node->SetRotation(Quaternion(90.0f));
                        node->CreateComponent<RigidBody2D>();
                        auto shape = node->CreateComponent<CollisionBox2D>();
                        shape->SetSize(Vector2(this->GetWindowWidth() / 2.0f, this->wallWidth));
                        shape->SetRestitution(0.0);
                    }
                }},
                {Main::sceneNameToIdx.at("hole-top"), [&](){
                    this->SetTitle("Topology");
                    this->CreateWallNodes();
                    this->CreateRandomPlayerNode();
                    this->CreateRandomAppleNode();
                    {
                        auto node = this->scene->CreateChild("SeparatorWall");
                        node->SetPosition2D(Vector2(this->GetWindowWidth() / 2.0, 3.0f * this->GetWindowHeight() / 8.0f));
                        node->SetRotation(Quaternion(90.0f));
                        node->CreateComponent<RigidBody2D>();
                        auto shape = node->CreateComponent<CollisionBox2D>();
                        shape->SetSize(Vector2(3.0f * this->GetWindowWidth() / 4.0f, this->wallWidth));
                        shape->SetRestitution(0.0);
                    }
                }},
                {Main::sceneNameToIdx.at("small-hole"), [&](){
                    this->SetTitle("I can't get through here");
                    this->CreateWallNodes();
                    this->CreatePlayerNode(Vector2(this->GetWindowWidth() / 4.0f, this->GetWindowHeight() / 2.0f), -90.0f);
                    this->CreateAppleNode(Vector2(3.0f * this->GetWindowWidth() / 4.0f, this->GetWindowHeight() / 2.0f));
                    auto bottomSeparatorWallNode = this->scene->CreateChild("SeparatorWallBottom");
                    {
                        auto& node = bottomSeparatorWallNode;
                        node->SetPosition2D(Vector2(
                            this->GetWindowWidth() / 2.0,
                            3.0f * this->GetWindowHeight() / 8.0f - Main::playerRadius / 2.0f
                        ));
                        node->SetRotation(Quaternion(90.0f));
                        node->CreateComponent<RigidBody2D>();
                        auto shape = node->CreateComponent<CollisionBox2D>();
                        shape->SetSize(Vector2(this->GetWindowWidth() / 4.0f, this->wallWidth));
                        shape->SetRestitution(0.0);
                    } {
                        auto node = bottomSeparatorWallNode->Clone();
                        node->SetName("SeparatorWallTop");
                        node->Translate2D(Vector2(2.0f * this->GetWindowHeight() / 8.0f + Main::playerRadius, 0.0f));
                    }
                }},
                {Main::sceneNameToIdx.at("patrol-door"), [&](){
                    this->SetTitle("Patience");
                    this->CreateWallNodes();
                    this->CreateRandomPlayerNode();
                    this->CreateRandomAppleNode();
                    auto bottomSeparatorWallNode = this->scene->CreateChild("SeparatorWallBottom");
                    auto bottomSeparatorLength = (this->GetWindowHeight() - 3.0f * Main::playerRadius) / 2.0f;
                    {
                        auto& node = bottomSeparatorWallNode;
                        node->SetPosition2D(Vector2(
                            this->GetWindowWidth() / 2.0f,
                            bottomSeparatorLength / 2.0f
                        ));
                        node->SetRotation(Quaternion(90.0f));
                        node->CreateComponent<RigidBody2D>();
                        auto shape = node->CreateComponent<CollisionBox2D>();
                        shape->SetSize(Vector2(bottomSeparatorLength, this->wallWidth));
                        shape->SetRestitution(0.0);
                    } {
                        auto node = bottomSeparatorWallNode->Clone();
                        node->SetName("SeparatorWallTop");
                        node->SetPosition2D(Vector2(
                            bottomSeparatorWallNode->GetPosition2D().x_,
                            this->GetWindowHeight() - bottomSeparatorLength / 2.0f
                        ));
                    } {
                        auto node = bottomSeparatorWallNode->Clone();
                        node->SetName("Door");
                        node->SetPosition2D(Vector2(
                            bottomSeparatorWallNode->GetPosition2D().x_,
                            this->GetWindowHeight() / 2.0f
                        ));
                        auto maxDistComponent = node->CreateComponent<MaxDistComponent>();
                        maxDistComponent->SetSpeed(Vector2::RIGHT * this->GetWindowWidth() / 4.0f);
                        maxDistComponent->SetMaxDist((this->GetWindowWidth() - bottomSeparatorLength) / 2.0f);
                        auto body = node->GetComponent<RigidBody2D>();
                        body->SetBodyType(BT_KINEMATIC);
                    }
                }},
                {Main::sceneNameToIdx.at("apple-button"), [&](){
                    this->SetTitle("What does this button do?");
                    this->CreateWallNodes();
                    this->CreateRandomPlayerNode();
                    auto buttonsNode = this->scene->CreateChild("Buttons");
                    auto appleButtonsAndComponent = buttonsNode->CreateComponent<AppleButtonsAndComponent>();
                    appleButtonsAndComponent->Init(this);
                    auto button = buttonsNode->CreateChild("button");
                    this->InitButtonNode(button);
                    this->MoveToRandomEmptySpace(button);
                    appleButtonsAndComponent->AddChildButton(button);
                }},
                {Main::sceneNameToIdx.at("apple-buttons-and"), [&](){
                    this->SetTitle("AND now there are two");
                    this->CreateWallNodes();
                    this->CreateRandomPlayerNode();
                    auto buttonsNode = this->scene->CreateChild("Buttons");
                    auto appleButtonsAndComponent = buttonsNode->CreateComponent<AppleButtonsAndComponent>();
                    appleButtonsAndComponent->Init(this);
                    {
                        auto button = this->scene->CreateChild("Button0");
                        this->InitButtonNode(button);
                        this->MoveToRandomEmptySpace(button);
                        appleButtonsAndComponent->AddChildButton(button);
                    }
                    {
                        auto button = this->scene->CreateChild("Button1");
                        this->InitButtonNode(button);
                        this->MoveToRandomEmptySpace(button);
                        appleButtonsAndComponent->AddChildButton(button);
                        this->MoveToRandomEmptySpace(button);
                    }
                }},
                {Main::sceneNameToIdx.at("button-door"), [&](){
                    this->SetTitle("Are buttons and doors related?");
                    this->CreateWallNodes();
                    this->CreateRandomPlayerNode();
                    auto bottomSeparatorWallNode = this->scene->CreateChild("SeparatorWallBottom");
                    auto bottomSeparatorLength = (this->GetWindowHeight() - 3.0f * Main::playerRadius) / 2.0f;
                    MaxDistComponent *maxDistComponent;
                    {
                        auto& node = bottomSeparatorWallNode;
                        node->SetPosition2D(Vector2(
                            this->GetWindowWidth() / 2.0f,
                            bottomSeparatorLength / 2.0f
                        ));
                        node->SetRotation(Quaternion(90.0f));
                        node->CreateComponent<RigidBody2D>();
                        auto shape = node->CreateComponent<CollisionBox2D>();
                        shape->SetSize(Vector2(bottomSeparatorLength, this->wallWidth));
                        shape->SetRestitution(0.0);
                    } {
                        auto node = bottomSeparatorWallNode->Clone();
                        node->SetName("SeparatorWallTop");
                        node->SetPosition2D(Vector2(
                            bottomSeparatorWallNode->GetPosition2D().x_,
                            this->GetWindowHeight() - bottomSeparatorLength / 2.0f
                        ));
                    } {
                        auto node = bottomSeparatorWallNode->Clone();
                        node->SetName("Door");
                        node->SetPosition2D(Vector2(
                            bottomSeparatorWallNode->GetPosition2D().x_,
                            this->GetWindowHeight() / 2.0f
                        ));
                        maxDistComponent = node->CreateComponent<MaxDistComponent>();
                        maxDistComponent->SetSpeed(Vector2::RIGHT * this->GetWindowWidth() / 4.0f);
                        maxDistComponent->SetMaxDist((this->GetWindowWidth() - bottomSeparatorLength) / 2.0f);
                        maxDistComponent->SetMaxBounces(1);
                        maxDistComponent->SetActive(false);
                        auto body = node->GetComponent<RigidBody2D>();
                        body->SetBodyType(BT_KINEMATIC);
                    }
                    {
                        auto button = this->scene->CreateChild("Button0");
                        this->InitButtonNode(button);
                        auto activateDoorButtonComponent = button->CreateComponent<ActivateDoorButtonComponent>();
                        this->MoveToRandomEmptySpace(
                            button,
                            Rect(0.0f, this->GetWindowHeight(), this->GetWindowWidth() / 2.0f, 0.0f)
                        );
                        activateDoorButtonComponent->Init(maxDistComponent);
                    }
                    {
                        auto button = this->scene->CreateChild("button");
                        this->InitButtonNode(button);
                        button->SetPosition(Vector2(3.0f * this->GetWindowWidth() / 4.0f, this->GetWindowHeight() / 4.0f));
                        auto activateDoorButtonComponent = button->CreateComponent<ActivateDoorButtonComponent>();
                        this->MoveToRandomEmptySpace(
                            button,
                            Rect(this->GetWindowWidth() / 2.0f, this->GetWindowHeight(), this->GetWindowWidth(), 0.0f)
                        );
                        activateDoorButtonComponent->Init(maxDistComponent);
                    }
                    this->CreateRandomAppleNode();
                }},
                {Main::sceneNameToIdx.at("rock"), [&](){
                    // TODO: if you use the rock to push the apple into a corner, the rock and apple can overlap.
                    // And worse, for some reason this can lead to multiple apples spawning afterwards.
                    // This does not happen however on the Box2D sandbox, so it should be solvable somehow:
                    // https://github.com/cirosantilli/Box2D/tree/EdgeTest-overlap
                    // This can be easily reproducible with mouse drag.
                    this->SetTitle("Rocks aren't good.");
                    this->CreateWallNodes();
                    this->CreatePlayerNode(Vector2(this->GetWindowWidth() / 4.0f, this->GetWindowHeight() / 2.0f));
                    this->CreateRandomAppleNode();
                    this->CreateRandomRockNode();
                }},
                {Main::sceneNameToIdx.at("spikes"), [&](){
                    this->SetTitle("And spikes are just plain bad.");
                    this->CreateWallNodes();
                    this->CreatePlayerNode(Vector2(this->GetWindowWidth() / 4.0f, this->GetWindowHeight() / 2.0f));
                    this->CreateRandomAppleNode();
                    auto node = this->scene->CreateChild("Spikes");
                    node->SetVar("TouchScoreChange", -1.0f);
                    auto body = node->CreateComponent<RigidBody2D>();
                    body->SetBodyType(BT_STATIC);
                    auto shape = node->CreateComponent<CollisionCircle2D>();
                    shape->SetRadius(Main::playerRadius);
                    shape->SetFriction(0.0f);
                    shape->SetRestitution(Main::playerRestitution);
                    this->MoveToRandomEmptySpace(node);
                    Main::SetSprite(node, this->resourceCache->GetResource<Sprite2D>("./spikes-full.png"));
                }},
                {Main::sceneNameToIdx.at("bouncer"), [&](){
                    this->SetTitle("Bouncer");
                    this->CreateWallNodes();
                    Node *node;
                    this->CreateBouncerNode(node);
                    this->MoveToRandomEmptySpace(node);
                    this->CreateRandomPlayerNode();
                    this->CreateRandomAppleNode();
                    this->CreateRandomRockNode();
                }},
                {Main::sceneNameToIdx.at("trash"), [&](){
                    this->SetTitle("Get those stones out of here");
                    this->CreateWallNodes();
                    Node *node;
                    {
                        this->CreateBouncerNode(node);
                        node->SetPosition(Vector2(this->wallWidth, this->wallWidth));
                        node->SetRotation(Quaternion(45.0f));
                    }
                    {
                        this->CreateBouncerNode(node);
                        node->SetPosition(Vector2(this->GetWindowWidth() - this->wallWidth, this->wallWidth));
                        node->SetRotation(Quaternion(45.0f));
                    }
                    {
                        this->CreateBouncerNode(node);
                        node->SetPosition(Vector2(this->GetWindowWidth() - this->wallWidth, this->GetWindowHeight() - this->wallWidth));
                        node->SetRotation(Quaternion(45.0f));
                    }
                    {
                        this->CreateBouncerNode(node);
                        node->SetPosition(Vector2(this->wallWidth, this->GetWindowHeight() - this->wallWidth));
                        node->SetRotation(Quaternion(45.0f));
                    }
                    this->CreateRandomPlayerNode();
                    node = this->scene->CreateChild("TrashCan");
                    node->SubscribeToEvent(node, E_NODEBEGINCONTACT2D, [&](StringHash eventType, VariantMap& eventData) {
                        using namespace NodeBeginContact2D;
                        auto otherNode = static_cast<Node*>(eventData[P_OTHERNODE].GetPtr());
                        auto variant = otherNode->GetVar("IsTrash");
                        if (variant != Variant::EMPTY) {
                            Node *apple;
                            this->CreateRandomAppleNode(apple, false);
                            this->SubscribeToEvent(apple, "Consumed", [&](StringHash eventType, VariantMap& eventData){
                                this->CreateRandomRockNode();
                            });
                            otherNode->Remove();
                        }
                    });
                    auto body = node->CreateComponent<RigidBody2D>();
                    body->SetBodyType(BT_STATIC);
                    auto shape = node->CreateComponent<CollisionCircle2D>();
                    shape->SetRadius(Main::playerRadius);
                    shape->SetFriction(0.0f);
                    shape->SetRestitution(Main::playerRestitution);
                    this->MoveToRandomEmptySpace(node);
                    Main::SetSprite(node, this->resourceCache->GetResource<Sprite2D>("./trash-can.png"));
                    this->CreateRandomRockNode();
                }},
            }[this->sceneIdx]();
        }
    }
    virtual void StartOnce() override {
        String sceneName;
        auto args = GetArguments();
        decltype(args.Size()) i = 0;
        while (i < args.Size()) {
            auto& arg = args[i];
            if (arg == "-zscene") {
                ++i;
                sceneName = args[i].CString();
            }
            ++i;
        }
        auto it = this->sceneNameToIdx.find(sceneName);
        if (it != this->sceneNameToIdx.end()) {
            this->sceneIdx = it->second;
        }
    }
private:

    class ActivateDoorButtonComponent : public Component {
        URHO3D_OBJECT(ActivateDoorButtonComponent, Component);
    public:
        ActivateDoorButtonComponent(Context* context) : Component(context) {}
        void Init(MaxDistComponent *maxDistComponent) {
            this->maxDistComponent = maxDistComponent;
            this->SubscribeToEvent(this->node_, E_NODEBEGINCONTACT2D, URHO3D_HANDLER(ActivateDoorButtonComponent, HandleNodeBeginContact2D));
        }
    private:
        MaxDistComponent *maxDistComponent;
        void HandleNodeBeginContact2D(StringHash eventType, VariantMap& eventData) {
            this->maxDistComponent->Reset();
        }
    };

    /**
     * Spawn a random apple when all (logic AND) given child buttons are hit.
     */
    class AppleButtonsAndComponent : public Component {
        URHO3D_OBJECT(AppleButtonsAndComponent, Component);
    public:
        AppleButtonsAndComponent(Context* context) : Component(context) {
            this->nButtonsHit = 0;
            this->active = true;
        }
        void Init(Main *main) {
            this->main = main;
        }
        void AddChildButton(Node *node) {
            this->buttonsHit[node] = false;
            this->SubscribeToEvent(node, E_NODEBEGINCONTACT2D, URHO3D_HANDLER(AppleButtonsAndComponent, HandleChildCollision));
        }
    private:
        Main *main;
        std::map<Node*,bool> buttonsHit;
        decltype(buttonsHit)::size_type nButtonsHit;
        bool active;
        void HandleAppleEaten(StringHash eventType, VariantMap& eventData) {
            auto appleNode = (static_cast<Node*>(this->GetEventSender()));
            this->UnsubscribeFromEvent(appleNode, "Consumed");
            this->nButtonsHit = 0;
            this->active = true;
            for (auto &entry : this->buttonsHit) {
                entry.second = false;
            }
        }
        void HandleChildCollision(StringHash eventType, VariantMap& eventData) {
            auto buttonNode = (static_cast<Node*>(this->GetEventSender()));
            auto &hit = this->buttonsHit.at(buttonNode);
            if (!hit) {
                this->nButtonsHit++;
                hit = true;
            }
            if (this->active && this->nButtonsHit == this->buttonsHit.size()) {
                Node *apple;
                this->main->CreateRandomAppleNode(apple, false);
                this->SubscribeToEvent(apple, "Consumed", URHO3D_HANDLER(AppleButtonsAndComponent, HandleAppleEaten));
                this->active = false;
            }
        }
    };

    struct ContactData {
        Vector2 position;
        float impulse;
    };

    static class _StaticConstructor {
        public:
            _StaticConstructor() {
                static std::vector<String> scenes = {
                    "tutorial",
                    "apple",
                    "golden-apple",
                    "rotten-apple",
                    "hole-top-bottom",
                    "hole-top",
                    "small-hole",
                    "patrol-door",
                    "apple-button",
                    "apple-buttons-and",
                    "button-door",
                    "rock",
                    "spikes",
                    "bouncer",
                    "trash",
                };
                decltype(scenes)::size_type i = 0;
                for (const auto& scene : scenes) {
                    sceneNameToIdx.emplace(scene, i);
                    ++i;
                }
            }
    } _staticConstructor;

    static constexpr float playerDensity = 1.0f;
    static constexpr float playerFriction = 1.0f;
    static constexpr float playerRadius = 1.0f;
    static constexpr float playerRestitution = 0.2f;
    static constexpr float wallWidth = playerRadius;
    static std::map<String,size_t> sceneNameToIdx;

    static void Rotate2D(Vector2& vec, float angle) {
        auto vec3 = Quaternion(angle) * Vector3(vec);
        vec = Vector2(vec3.x_, vec3.y_);
    }

    static void SetSprite(Node *node, Sprite2D *sprite) {
        auto position = node->GetPosition2D();
        auto b2Aabb = node->GetDerivedComponent<CollisionShape2D>()->GetFixture()->GetAABB(0);
        auto lowerBound = Vector2(b2Aabb.lowerBound.x, b2Aabb.lowerBound.y);
        auto upperBound = Vector2(b2Aabb.upperBound.x, b2Aabb.upperBound.y);
        PODVector<RigidBody2D*> rigidBodies;
        auto aabb = Rect(lowerBound, upperBound);
        auto staticSprite = node->CreateComponent<StaticSprite2D>();
        staticSprite->SetSprite(sprite);
        staticSprite->SetDrawRect(aabb - Rect(position, position));
    }

    Node *playerNode;
    RigidBody2D *playerBody;
    Text *text;
    float score;
    float windowWidth;
    size_t sceneIdx;
    std::map<Node*,std::map<Node*,std::vector<ContactData>>> contactDataMap;

    size_t AabbCount(CollisionShape2D *shape) {
        // TODO use triggers instead of aabb to be more precise.
        // But is it possible without stepping the simulation?
        auto b2Aabb = shape->GetFixture()->GetAABB(0);
        auto lowerBound = Vector2(b2Aabb.lowerBound.x, b2Aabb.lowerBound.y);
        auto upperBound = Vector2(b2Aabb.upperBound.x, b2Aabb.upperBound.y);
        PODVector<RigidBody2D*> rigidBodies;
        auto aabb = Rect(lowerBound, upperBound);
        this->physicsWorld->GetRigidBodies(rigidBodies, aabb);
        if (false) {
            for (const auto& body : rigidBodies) {
                std::cout << body->GetNode()->GetName().CString() << std::endl;
            }
            std::cout << std::endl;
        }
        return rigidBodies.Size();
    }

    void CreateAppleNodeBase(Node *&node, bool respawn = true) {
        node = this->scene->CreateChild("Apple");
        node->SetVar("IsConsumable", true);
        node->SetVar(RESPAWN, respawn);
        auto body = node->CreateComponent<RigidBody2D>();
        body->SetBullet(true);
        body->SetBodyType(BT_DYNAMIC);
        body->SetLinearDamping(4.0);
        body->SetAngularDamping(4.0);
        auto shape = node->CreateComponent<CollisionCircle2D>();
        shape->SetRadius(Main::playerRadius);
        // TODO: make player not loose speed when hitting the apple.
        // The player is still pushed back when eating. Why.
        //shape->SetDensity(0.0f);
        // Works, until we reach cases where some things can apply forces to apples,
        // and then this would be just too light, the apple would fly away.
        //shape->SetDensity(1e-06f);
        // Prevents other things besides player, e.g. a rock, from interacting with the apple.
        //shape->SetTrigger(true);
        shape->SetDensity(Main::playerDensity);
        shape->SetFriction(0.0f);
        shape->SetRestitution(Main::playerRestitution);
    }

    void CreateAppleNode(Node *&node, bool respawn = true) {
        this->CreateAppleNodeBase(node, respawn);
        node->SetVar("TouchScoreChange", 1.0f);
        Main::SetSprite(node, this->resourceCache->GetResource<Sprite2D>("./shiny-apple-red.png"));
    }

    void CreateGoldenAppleNode(Node *&node, bool respawn = true) {
        this->CreateAppleNodeBase(node, respawn);
        node->SetVar("TouchScoreChange", 5.0f);
        Main::SetSprite(node, this->resourceCache->GetResource<Sprite2D>("./shiny-apple-yellow.png"));
    }

    void CreateRottenAppleNode(Node *&node, bool respawn = true) {
        this->CreateAppleNodeBase(node, respawn);
        node->SetVar("TouchScoreChange", -1.0f);
        Main::SetSprite(node, this->resourceCache->GetResource<Sprite2D>("./shiny-apple-brown.png"));
    }

    void CreateAppleNode(
        const Vector2& position,
        float rotation = 0.0f,
        bool respawn = true
    ) {
        Node *node;
        this->CreateAppleNode(node, position, rotation, respawn);
    }

    void CreateAppleNode(
        Node *&node,
        const Vector2& position,
        float rotation = 0.0f,
        bool respawn = true
    ) {
        this->CreateAppleNode(node, respawn);
        node->SetPosition2D(position);
        node->SetRotation(Quaternion(rotation));
    }

    void CreateBouncerNode(Node *&node) {
        node = this->scene->CreateChild("Bouncer");
        auto body = node->CreateComponent<RigidBody2D>();
        body->SetBodyType(BT_STATIC);
        auto shape = node->CreateComponent<CollisionBox2D>();
        shape->SetSize(2.0f * Main::playerRadius * Vector2::ONE);
        shape->SetFriction(0.0f);
        shape->SetRestitution(Main::playerRestitution);
        Main::SetSprite(node, this->resourceCache->GetResource<Sprite2D>("./move.png"));
        node->SubscribeToEvent(node, E_NODEBEGINCONTACT2D, [](StringHash eventType, VariantMap& eventData) {
            using namespace NodeBeginContact2D;
            auto otherBody = static_cast<RigidBody2D*>(eventData[P_OTHERBODY].GetPtr());
            MemoryBuffer contacts(eventData[P_CONTACTS].GetBuffer());
            /*auto position = */contacts.ReadVector2();
            auto normal = contacts.ReadVector2();
            otherBody->ApplyLinearImpulseToCenter(100.0f * normal, true);
        });
    }

    void CreatePlayerNode() {
        auto& node = this->playerNode;
        node = this->scene->CreateChild("Player");
        auto& body = this->playerBody;
        body = node->CreateComponent<RigidBody2D>();
        body->SetBodyType(BT_DYNAMIC);
        body->SetLinearDamping(4.0);
        body->SetAngularDamping(4.0);
        body->SetBullet(true);
        auto shape = node->CreateComponent<CollisionCircle2D>();
        shape->SetDensity(Main::playerDensity);
        shape->SetFriction(0.0f);
        shape->SetRadius(Main::playerRadius);
        shape->SetRestitution(Main::playerRestitution);
        this->CreateCamera(node, 20.0f * playerRadius);
        Main::SetSprite(node, this->resourceCache->GetResource<Sprite2D>("./baby-face.png"));
    }

    void CreatePlayerNode(const Vector2& position, float rotation = 0.0f) {
        this->CreatePlayerNode();
        auto& node = this->playerNode;
        node->SetPosition2D(position);
        node->SetRotation(Quaternion(rotation));
    }

    void CreateRandomAppleNode(bool respawn = true) {
        Node *node;
        this->CreateRandomAppleNode(node, respawn);
    }

    void CreateRandomAppleNode(Node *&node, bool respawn = true) {
        this->CreateAppleNode(node, respawn);
        this->MoveToRandomEmptySpace(node);
    }

    void CreateRandomGoldenAppleNode() {
        Node *node;
        this->CreateGoldenAppleNode(node);
        this->MoveToRandomEmptySpace(node);
    }

    void CreateRandomRottenAppleNode() {
        Node *node;
        this->CreateRottenAppleNode(node);
        this->MoveToRandomEmptySpace(node);
    }

    void CreateRandomRockNode() {
        Node *node;
        this->CreateRockNode(node);
        this->MoveToRandomEmptySpace(node);
    }

    void CreateRandomPlayerNode() {
        this->CreatePlayerNode();
        this->MoveToRandomEmptySpace(this->playerNode);
    }

    void CreateRockNode(Node *&node) {
        node = this->scene->CreateChild("Rock");
        node->SetVar("IsTrash", true);
        auto body = node->CreateComponent<RigidBody2D>();
        body->SetBodyType(BT_DYNAMIC);
        body->SetBullet(true);
        body->SetLinearDamping(4.0);
        body->SetAngularDamping(4.0);
        auto shape = node->CreateComponent<CollisionCircle2D>();
        shape->SetDensity(Main::playerDensity);
        shape->SetFriction(0.0f);
        shape->SetRadius(Main::playerRadius);
        shape->SetRestitution(Main::playerRestitution);
        Main::SetSprite(node, this->resourceCache->GetResource<Sprite2D>("./rock.png"));
    }

    void CreateWallNodes() {
        Node *bottomWallNode;
        {
            bottomWallNode = this->scene->CreateChild("BottomWall");
            bottomWallNode->SetPosition2D(Vector2(this->GetWindowWidth() / 2.0, this->wallWidth / 2.0f));
            bottomWallNode->CreateComponent<RigidBody2D>();
            auto shape = bottomWallNode->CreateComponent<CollisionBox2D>();
            shape->SetSize(Vector2(this->GetWindowWidth(), this->wallWidth));
            shape->SetRestitution(0.0);
        } {
            auto node = bottomWallNode->Clone();
            node->SetName("TopWall");
            node->SetPosition2D(Vector2(this->GetWindowWidth() / 2.0f, this->GetWindowHeight() - (this->wallWidth / 2.0f)));
        } {
            auto node = bottomWallNode->Clone();
            node->SetName("RightWall");
            node->SetRotation(Quaternion(90.0f));
            node->SetPosition2D(Vector2(this->GetWindowWidth() - (this->wallWidth / 2.0f), this->GetWindowHeight() / 2.0f));
        } {
            auto node = bottomWallNode->Clone();
            node->SetName("LeftWall");
            node->SetRotation(Quaternion(90.0f));
            node->SetPosition2D(Vector2(this->wallWidth / 2.0f, this->GetWindowHeight() / 2.0f));
        }
    }

    virtual float GetWindowWidth() const override { return this->windowWidth; }

    virtual void HandleKeyDownExtra(StringHash eventType, VariantMap& eventData) override {
        using namespace KeyDown;
        auto key = eventData[P_KEY].GetInt();
        if (key == KEY_N) {
            this->sceneIdx = (this->sceneIdx + 1) % this->sceneNameToIdx.size();
            this->Reset();
        } else if (key == KEY_P) {
            this->sceneIdx = (this->sceneIdx == 0 ? this->sceneNameToIdx.size() : this->sceneIdx) - 1;
            this->Reset();
        }
    }

    virtual void HandlePhysicsBeginContact2DExtra(StringHash eventType, VariantMap& eventData) override {
        using namespace PhysicsBeginContact2D;
        auto nodea = static_cast<Node*>(eventData[P_NODEA].GetPtr());
        auto nodeb = static_cast<Node*>(eventData[P_NODEB].GetPtr());
        Node *otherNode;
        bool player = false;
        if (nodea == this->playerNode) {
            otherNode = nodeb;
            player = true;
        }
        if (nodeb == this->playerNode) {
            otherNode = nodea;
            player = true;
        }
        if (player) {
            {
                auto variant = otherNode->GetVar("TouchScoreChange");
                if (variant != Variant::EMPTY) {
                    this->SetScore(this->score + variant.GetFloat());
                }
            }
            {
                auto variant = otherNode->GetVar("IsConsumable");
                if (variant != Variant::EMPTY && variant.GetBool()) {
                    VariantMap eventData;
                    eventData["SENDER"] = this;
                    otherNode->SendEvent("Consumed", eventData);
                    if (otherNode->GetVar(RESPAWN).GetBool()) {
                        this->MoveToRandomEmptySpace(otherNode);
                        auto body = otherNode->GetComponent<RigidBody2D>();
                        body->SetLinearVelocity(Vector2::ZERO);
                        body->SetAngularVelocity(0.0f);
                    } else {
                        otherNode->Remove();
                    }
                }
            }
        }
    }

    virtual void HandlePhysicsUpdateContact2DExtra(StringHash eventType, VariantMap& eventData) override {
        using namespace PhysicsUpdateContact2D;
        auto nodea = static_cast<Node*>(eventData[P_NODEA].GetPtr());
        auto nodeb = static_cast<Node*>(eventData[P_NODEB].GetPtr());
        MemoryBuffer contacts(eventData[P_CONTACTS].GetBuffer());
        while (!contacts.IsEof()) {
            auto position = contacts.ReadVector2();
            auto normal = contacts.ReadVector2();
            auto distance = contacts.ReadFloat();
            auto impulse = contacts.ReadFloat();
            // TODO shared pointer here.
            this->contactDataMap[nodea][nodeb].push_back(ContactData{position, impulse});
            this->contactDataMap[nodeb][nodea].push_back(ContactData{position, impulse});
            if (false) {
                std::cout << "contact position " << position.ToString().CString() << std::endl;
                std::cout << "contact normal " << normal.ToString().CString() << std::endl;
                std::cout << "contact distance " << distance << std::endl;
                std::cout << "contact impulse " << impulse << std::endl;
                std::cout << std::endl;
            }
        }
    }

    virtual void HandleUpdateExtra(StringHash eventType, VariantMap& eventData) override {
        using namespace Update;

        auto playerPosition = this->playerNode->GetPosition2D();
        auto playerRotation = this->playerNode->GetRotation2D();
        Vector2 playerForwardDirection = Vector2::UP;
        this->Rotate2D(playerForwardDirection, playerRotation);

        // Camera sensor
        std::vector<PhysicsRaycastResult2D> raycastResults;
        auto nrays = 8u;
        auto angleStep = -360.0f / nrays;
        for (auto i = 0u; i < nrays; ++i) {
            auto angle = i * angleStep;
            auto direction = playerForwardDirection;
            this->Rotate2D(direction, angle);
            auto position = this->playerNode->GetPosition2D();
            auto startPoint = position + (Main::playerRadius * direction);
            auto endPoint = position + (2.0f * this->GetWindowWidth() * direction);
            PhysicsRaycastResult2D result;
            this->physicsWorld->RaycastSingle(result, startPoint, endPoint);
            raycastResults.push_back(result);
        }
        if (false) {
            for (const auto& result : raycastResults) {
                auto body = result.body_;
                if (body == nullptr) {
                    std::cout << "nullptr" << std::endl;
                } else {
                    std::cout << body->GetNode()->GetName().CString() << " " << result.distance_ << std::endl;
                }
            }
            std::cout << std::endl;
        }

        // Touch sensor.
        for (const auto& keyVal : contactDataMap[this->playerNode]) {
            auto node = keyVal.first;
            auto contactDatas = keyVal.second;
            for (const auto& contactData : contactDatas) {
                auto contactDirection = contactData.position - playerPosition;
                contactDirection.Normalize();
                auto contactAngle = Atan2(playerForwardDirection.y_, playerForwardDirection.x_) - Atan2(contactDirection.y_, contactDirection.x_);
                if (false) {
                    std::cout << "name: " << node->GetName().CString() << std::endl;
                    std::cout << "position: " << contactData.position.ToString().CString() << std::endl;
                    std::cout << "angle: " << contactAngle << std::endl;
                    std::cout << "impulse: " << contactData.impulse << std::endl;
                    std::cout << std::endl;
                }
            }
        }

#if 0
        // This should be avoided, components should be favored.
        // Scene logic
        {
            static const std::unordered_map<size_t,std::function<void(Main*)>> m{
                {Main::sceneNameToIdx["door-patrol"], [&](Main* thiz){
                    auto door = thiz->scene->GetChild("Door");
                }}
            };
            static const auto end = m.end();
            auto it = m.find(this->sceneIdx);
            if (it != end) {
                it->second(this);
            }
        }
#endif

        // Act
        {
            auto playerBody = this->playerNode->GetComponent<RigidBody2D>();
            auto playerMass = Main::playerDensity * Main::playerRadius;

            // Linear movement
            {
                auto forceMagnitude = 500.0f * playerMass;
                if (this->input->GetKeyDown(KEY_S)) {
                    Vector2 direction = playerForwardDirection;
                    this->Rotate2D(direction, 180.0);
                    playerBody->ApplyForceToCenter(direction * forceMagnitude, true);
                }
                if (this->input->GetKeyDown(KEY_W)) {
                    Vector2 direction = playerForwardDirection;
                    playerBody->ApplyForceToCenter(direction * forceMagnitude, true);
                }
                if (this->input->GetKeyDown(KEY_A)) {
                    Vector2 direction = playerForwardDirection;
                    this->Rotate2D(direction, 90.0);
                    playerBody->ApplyForceToCenter(direction * forceMagnitude, true);
                }
                if (this->input->GetKeyDown(KEY_D)) {
                    Vector2 direction = playerForwardDirection;
                    this->Rotate2D(direction, -90.0);
                    playerBody->ApplyForceToCenter(direction * forceMagnitude, true);
                }
            }

            // Rotate
            auto torqueMagnitude = 20.0f * playerMass;
            if (this->input->GetKeyDown(KEY_Q)) {
                playerBody->ApplyTorque(torqueMagnitude, true);
            }
            if (this->input->GetKeyDown(KEY_E)) {
                playerBody->ApplyTorque(-torqueMagnitude, true);
            }
        }

#if 0
        this->SetScore(this->score - 0.001f);
#endif
        contactDataMap.clear();
    }

    void InitButtonNode(Node *node) {
        auto body = node->CreateComponent<RigidBody2D>();
        auto shape = node->CreateComponent<CollisionCircle2D>();
        shape->SetDensity(Main::playerDensity);
        shape->SetFriction(0.0f);
        shape->SetRadius(Main::playerRadius);
        shape->SetRestitution(Main::playerRestitution);
        Main::SetSprite(node, this->resourceCache->GetResource<Sprite2D>("./button-finger.png"));
    }

    void MoveToRandomEmptySpace(Node *node) {
        Rect bounds(0.0f, this->GetWindowHeight(), this->GetWindowWidth(), 0.0f);
        this->MoveToRandomEmptySpace(node, bounds);
    }

    void MoveToRandomEmptySpace(Node *node, const Rect& bounds) {
        do {
            node->SetPosition(Vector2(
                bounds.Left()   + (Random() * (bounds.Right() - bounds.Left())),
                bounds.Bottom() + (Random() * (bounds.Top()   - bounds.Bottom()))
            ));
            node->SetRotation(Quaternion(Random() * 360.0f));
        } while (this->AabbCount(node->GetDerivedComponent<CollisionShape2D>()) > 1);
    }

    void SetScore(float score) {
        this->score = score;
        std::stringstream ss;
        ss << "Score: " << std::fixed << std::setprecision(0) << score;
        this->text->SetText(ss.str().c_str());
    }

    void SetTitle(String title) {
        auto text = this->ui->GetRoot()->CreateChild<Text>();
        text->SetFont(this->font, 20);
        text->SetAlignment(HA_CENTER, VA_TOP);
        text->SetPosition(0, 10);
        std::stringstream ss;
        ss << this->sceneIdx << " " << title.CString();
        text->SetText(ss.str().c_str());
    }
};

std::map<String,size_t> Main::sceneNameToIdx;
Main::_StaticConstructor Main::_staticConstructor;

URHO3D_DEFINE_APPLICATION_MAIN(Main);
