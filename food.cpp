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
        this->windowWidth = 20.0f * Main::playerRadius;
        this->drawDebugGeometry = false;
        context->RegisterFactory<ActivateDoorButtonComponent>();
        context->RegisterFactory<AppleButtonsAndComponent>();
        context->RegisterFactory<HumanActorComponent>();
        context->RegisterFactory<MaxDistComponent>();
        context->RegisterFactory<PlayerComponent>();
        context->RegisterFactory<RottenAppleSpawnerComponent>();
    }
    virtual void StartExtra() override {

        // Application state.
        this->text = this->ui->GetRoot()->CreateChild<Text>();
        this->text->SetFont(this->font, 20);
        this->text->SetAlignment(HA_RIGHT, VA_BOTTOM);
        this->text->SetPosition(-10, -10);

        // Scene
        {
            std::unordered_map<size_t,std::function<void()>> {
                {Main::sceneNameToIdx.at("tutorial"), [&](){
                    this->SetTitle("Tutorial");
                    this->CreateWallNodes();
                    this->CreateRandomPlayerNode();
                    auto text = this->ui->GetRoot()->CreateChild<Text>();
                    text->SetFont(this->font, 20);
                    text->SetAlignment(HA_CENTER, VA_CENTER);
                    text->SetText(
                        "ASDW: the usual\n"
                        "Q: turn left\n"
                        "E: turn right\n"
                        "N: Next scene\n"
                        "P: Previous scene\n"
                        "ESC: quit\n"
                    );
                }},
                {Main::sceneNameToIdx.at("apple"), [&](){
                    // Introduce the player to apples.
                    this->SetTitle("Apples are good");
                    this->CreateWallNodes();
                    this->CreateRandomPlayerNode();
                    this->CreateRandomAppleNode();
                }},
                {Main::sceneNameToIdx.at("golden-apple"), [&](){
                    // Introduce the player to golden apples.
                    this->SetTitle("Golden apples are better");
                    this->CreateWallNodes();
                    this->CreateRandomPlayerNode();
                    this->CreateRandomGoldenAppleNode();
                    this->CreateRandomAppleNode();
                }},
                {Main::sceneNameToIdx.at("rotten-apple"), [&](){
                    // Introduce the player to rotten apples.
                    this->SetTitle("Rotten apples are bad");
                    this->CreateWallNodes();
                    this->CreateRandomPlayerNode();
                    this->CreateRandomAppleNode();
                    this->CreateRandomRottenAppleNode();
                }},
                {Main::sceneNameToIdx.at("no-pain"), [&](){
                    // The player must get through a transparent wall of rotten apples to reach a golden apple.
                    this->SetTitle("No pain");
                    this->CreateWallNodes();
                    Node *node;
                    for (float y = this->wallWidth + 1.5f * Main::playerRadius; y < this->GetWindowHeight(); y += 3.0f * Main::playerRadius) {
                        node = this->scene->CreateChild("RottenAppleSpawner");
                        node->SetPosition2D(Vector2(this->GetWindowWidth() / 2.0f, y));
                        auto component = node->CreateComponent<RottenAppleSpawnerComponent>();
                        component->Init(this);
                    }
                    this->CreateRandomPlayerNode();
                    this->CreateRandomGoldenAppleNode();
                }},
                {Main::sceneNameToIdx.at("hole-top-bottom"), [&](){
                    // Minimal maze. The player must memorize the wall locations to be efficient.
                    // Requires a concept of curiosity on the bot, since apple is not seen initially.
                    this->SetTitle("I don't see no apple");
                    this->CreateWallNodes();
                    this->CreatePlayerNode(Vector2(this->GetWindowWidth() / 4.0f, this->GetWindowHeight() / 2.0f), -90.0f);
                    this->CreateAppleNode(Vector2(3.0f * this->GetWindowWidth() / 4.0f, this->GetWindowHeight() / 2.0f));
                    {
                        Node *node;
                        this->CreateWallNode(node, this->GetWindowWidth() / 2.0f);
                        node->SetName("SeparatorWall");
                        node->SetPosition2D(Vector2(this->GetWindowWidth() / 2.0, this->GetWindowHeight() / 2.0f));
                        node->SetRotation(Quaternion(90.0f));
                    }
                }},
                {Main::sceneNameToIdx.at("hole-top"), [&](){
                    // Another maze, but with a single door.
                    this->SetTitle("Topology");
                    this->CreateWallNodes();
                    this->CreateRandomPlayerNode();
                    this->CreateRandomAppleNode();
                    {
                        auto node = this->scene->CreateChild("SeparatorWall");
                        node->CreateComponent<RigidBody2D>();
                        auto shape = node->CreateComponent<CollisionBox2D>();
                        shape->SetSize(Vector2(3.0f * this->GetWindowWidth() / 4.0f, this->wallWidth));
                        shape->SetRestitution(0.0);
                        Main::SetSprite(node, this->resourceCache->GetResource<Sprite2D>("./gray.png"));
                        node->SetPosition2D(Vector2(this->GetWindowWidth() / 2.0, 3.0f * this->GetWindowHeight() / 8.0f));
                        node->SetRotation(Quaternion(90.0f));
                    }
                }},
                {Main::sceneNameToIdx.at("small-hole"), [&](){
                    // The player can an apple through a hole,
                    // but the hole is too small to get through.
                    this->SetTitle("I need a diet");
                    this->CreateWallNodes();
                    this->CreatePlayerNode(Vector2(this->GetWindowWidth() / 4.0f, this->GetWindowHeight() / 2.0f), -90.0f);
                    this->CreateAppleNode(Vector2(3.0f * this->GetWindowWidth() / 4.0f, this->GetWindowHeight() / 2.0f));
                    auto bottomSeparatorWallNode = this->scene->CreateChild("SeparatorWallBottom");
                    {
                        auto& node = bottomSeparatorWallNode;
                        node->SetRotation(Quaternion(90.0f));
                        node->SetPosition2D(Vector2(
                            this->GetWindowWidth() / 2.0,
                            3.0f * this->GetWindowHeight() / 8.0f - Main::playerRadius / 2.0f
                        ));
                        auto shape = node->CreateComponent<CollisionBox2D>();
                        shape->SetSize(Vector2(this->GetWindowWidth() / 4.0f, this->wallWidth));
                        shape->SetRestitution(0.0);
                        node->CreateComponent<RigidBody2D>();
                        Main::SetSprite(node, this->resourceCache->GetResource<Sprite2D>("./gray.png"));
                    } {
                        auto node = bottomSeparatorWallNode->Clone();
                        node->SetName("SeparatorWallTop");
                        node->Translate2D(Vector2(2.0f * this->GetWindowHeight() / 8.0f + Main::playerRadius, 0.0f));
                    }
                }},
                {Main::sceneNameToIdx.at("patrol-door"), [&](){
                    // A door opens and closes by itself periodically.
                    // The player must wait for opening to go to other half of the room.
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
                        Main::SetSprite(node, this->resourceCache->GetResource<Sprite2D>("./gray.png"));
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
                        auto shape = node->GetComponent<CollisionBox2D>();
                        shape->SetSize(Vector2(bottomSeparatorLength, this->wallWidth * 0.9f));
                        auto maxDistComponent = node->CreateComponent<MaxDistComponent>();
                        maxDistComponent->SetSpeed(Vector2::RIGHT * this->GetWindowWidth() / 4.0f);
                        maxDistComponent->SetMaxDist((this->GetWindowWidth() - bottomSeparatorLength) / 2.0f);
                        auto body = node->GetComponent<RigidBody2D>();
                        body->SetBodyType(BT_KINEMATIC);
                        Main::SetSprite(node, this->resourceCache->GetResource<Sprite2D>("./brown.png"), -1);
                    }
                }},
                {Main::sceneNameToIdx.at("apple-button"), [&](){
                    // An apple appears then the button is pressed.
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
                    // Both buttons must be pressed, one after the other, for an apple to appear.
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
                    // The door opens when the button is pressed.
                    // If there are no apples on this side of the wall,
                    // the player must open the door to explore the other side.
                    // TODO: the a button could block the door and make passage impossible.
                    this->SetTitle("Are buttons and doors related?");
                    this->CreateWallNodes();
                    this->CreateRandomPlayerNode();
                    Node *bottomSeparatorWallNode;
                    auto bottomSeparatorLength = (this->GetWindowHeight() - 3.0f * Main::playerRadius) / 2.0f;
                    MaxDistComponent *maxDistComponent;
                    {
                        auto& node = bottomSeparatorWallNode;
                        this->CreateWallNode(node, bottomSeparatorLength);
                        node->SetName("SeparatorWallBottom");
                        node->SetPosition2D(Vector2(
                            this->GetWindowWidth() / 2.0f,
                            bottomSeparatorLength / 2.0f
                        ));
                        node->SetRotation(Quaternion(90.0f));
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
                        Main::SetSprite(node, this->resourceCache->GetResource<Sprite2D>("./brown.png"), -1);
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
                    // Introduce the player to rocks.
                    //
                    // TODO: if you use the rock to push the apple into a corner, the rock and apple can overlap.
                    // And worse, for some reason this can lead to multiple apples spawning afterwards.
                    // This does not happen however on the Box2D sandbox, so it should be solvable somehow:
                    // https://github.com/cirosantilli/Box2D/tree/EdgeTest-overlap
                    // This can be easily reproducible with mouse drag.
                    this->SetTitle("Rocks aren't good.");
                    this->CreateWallNodes();
                    this->CreateRandomPlayerNode();
                    this->CreateRandomAppleNode();
                    this->CreateRandomRockNode();
                }},
                {Main::sceneNameToIdx.at("spikes"), [&](){
                    // Introduce the player to spikes.
                    this->SetTitle("And spikes are just plain bad.");
                    this->CreateWallNodes();
                    this->CreateRandomPlayerNode();
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
                    Main::SetSprite(node, this->resourceCache->GetResource<Sprite2D>("./spikes.png"));
                }},
                {Main::sceneNameToIdx.at("bouncer"), [&](){
                    // Introduce the player to a bouncer.
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
                    // The player must put the stones in the trashcan to get an apple.
                    this->SetTitle("Get those stones out of here");
                    this->CreateWallNodes();
                    this->CreateCornerBouncers();
                    this->CreateRandomPlayerNode();
                    {
                        auto node = this->scene->CreateChild("TrashCan");
                        node->SubscribeToEvent(node, E_NODEBEGINCONTACT2D, [&](StringHash eventType, VariantMap& eventData) {
                            using namespace NodeBeginContact2D;
                            auto otherNode = static_cast<Node*>(eventData[P_OTHERNODE].GetPtr());
                            if (otherNode->GetVar("IsTrash") != Variant::EMPTY) {
                                otherNode->Remove();
                                Node *apple;
                                this->CreateRandomAppleNode(apple, false);
                                this->SubscribeToEvent(apple, "Consumed", [&](StringHash eventType, VariantMap& eventData){
                                    this->CreateRandomRockNode();
                                });
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
                    }
                    this->CreateRandomRockNode();
                }},
                {Main::sceneNameToIdx.at("competition"), [&](){
                    // Two players fight for one apple.
                    this->SetTitle("We are not alone");
                    this->CreateWallNodes();
                    this->CreateRandomPlayerNode(false);
                    this->playerNode->GetComponent<HumanActorComponent>()->Init2();
                    this->CreateRandomPlayerNode(false);
                    this->CreateRandomAppleNode();
                    auto text = this->ui->GetRoot()->CreateChild<Text>();
                    text->SetFont(this->font, 20);
                    text->SetAlignment(HA_CENTER, VA_CENTER);
                    text->SetText(
                        "I: forward\n"
                        "U: turn left\n"
                        "O: turn right\n"
                    );
                }},
                {Main::sceneNameToIdx.at("collaboration"), [&](){
                    // If either player gets the apple, both get a point.
                    this->SetTitle("Collaboration");
                    this->CreateWallNodes();
                    Node *player1, *player2;
                    {
                        this->CreatePlayerNode(player2, false);
                        player2->GetComponent<HumanActorComponent>()->Init2();
                        this->CreatePlayerNode(player1, false);
                        this->playerNode = player1;
                        this->MoveToRandomEmptySpace(player1, false);
                        this->MoveToRandomEmptySpace(player2, false);
                    }
                    {
                        Node *apple;
                        this->CreateAppleNodeBase(apple);
                        Main::SetSprite(apple, this->resourceCache->GetResource<Sprite2D>("./shiny-apple-red.png"));
                        this->MoveToRandomEmptySpace(apple);
                        apple->SubscribeToEvent(apple, E_NODEBEGINCONTACT2D, [player1,player2](StringHash eventType, VariantMap& eventData) {
                            using namespace NodeBeginContact2D;
                            auto otherNode = static_cast<Node*>(eventData[P_OTHERNODE].GetPtr());
                            if (otherNode->GetComponent<PlayerComponent>()) {
                                player1->GetComponent<PlayerComponent>()->IncrementScore(1.0f);
                                player2->GetComponent<PlayerComponent>()->IncrementScore(1.0f);
                            }
                        });
                    }
                }},
                {Main::sceneNameToIdx.at("collabotition"), [&](){
                    // Both player must touch button at same time for apple to appear.
                    //
                    // When buttonsa re asymmetric, this creates a prisonner's dillema,
                    // since the player touching the middle button can almost always get the apple.
                    //
                    // TODO: if buttons are too close, a single player an get the apples.
                    this->SetTitle("collabotition");
                    this->CreateWallNodes();
                    this->CreateRandomPlayerNode(false);
                    this->playerNode->GetComponent<HumanActorComponent>()->Init2();
                    this->CreateRandomPlayerNode(false);
                    auto buttonsNode = this->scene->CreateChild("Buttons");
                    auto appleButtonsAndComponent = buttonsNode->CreateComponent<AppleButtonsAndComponent>();
                    appleButtonsAndComponent->Init(this, true);
                    {
                        auto button = this->scene->CreateChild("Button0");
                        this->InitButtonNode(button);
                        this->MoveToRandomEmptySpace(button, false);
                        appleButtonsAndComponent->AddChildButton(button);
                    }
                    {
                        auto button = this->scene->CreateChild("Button1");
                        this->InitButtonNode(button);
                        this->MoveToRandomEmptySpace(button, false);
                        appleButtonsAndComponent->AddChildButton(button);
                    }
                }},
                {Main::sceneNameToIdx.at("basketball"), [&](){
                    // When a player scores, it gains 1 point, and the other player loses 1 point.
                    this->SetTitle("Basketrock");
                    this->CreateWallNodes();
                    this->CreateCornerBouncers();
                    Node *player1, *player2;
                    this->CreatePlayerNode(player2, false);
                    player2->GetComponent<HumanActorComponent>()->Init2();
                    this->CreatePlayerNode(player1, false);
                    this->playerNode = player1;
                    auto createBasket = [this](Node *player1, Node *player2, const Vector2& position) {
                        auto node = this->scene->CreateChild("Basket");
                        node->SubscribeToEvent(node, E_NODEBEGINCONTACT2D, [this,player1,player2](StringHash eventType, VariantMap& eventData) {
                            using namespace NodeBeginContact2D;
                            auto otherNode = static_cast<Node*>(eventData[P_OTHERNODE].GetPtr());
                            if (otherNode->GetVar("IsTrash") != Variant::EMPTY) {
                                this->MoveToRandomEmptySpace(otherNode);
                                player1->GetComponent<PlayerComponent>()->IncrementScore(-1.0f);
                                player2->GetComponent<PlayerComponent>()->IncrementScore(1.0f);
                            }
                        });
                        node->SetPosition(position);
                        auto body = node->CreateComponent<RigidBody2D>();
                        body->SetBodyType(BT_STATIC);
                        auto shape = node->CreateComponent<CollisionCircle2D>();
                        shape->SetRadius(Main::playerRadius);
                        shape->SetFriction(0.0f);
                        shape->SetRestitution(Main::playerRestitution);
                        Main::SetSprite(node, this->resourceCache->GetResource<Sprite2D>("./trash-can.png"));
                    };
                    createBasket(player1, player2, Vector2(Main::wallWidth + Main::playerRadius, this->GetWindowHeight() / 2.0f));
                    createBasket(player2, player1, Vector2(this->GetWindowWidth() - (Main::wallWidth + Main::playerRadius), this->GetWindowHeight() / 2.0f));
                    this->MoveToRandomEmptySpace(player1, false);
                    this->MoveToRandomEmptySpace(player2, false);
                    this->CreateRandomRockNode();
                }},
            }[this->sceneIdx]();
        }
    }
    virtual void StartOnce() override {
        String sceneName;
        auto args = GetArguments();
        size_t sceneIdx;
        bool sceneIdxGiven = false;
        decltype(args.Size()) i = 0;
        while (i < args.Size()) {
            auto& arg = args[i];
            if (arg == "-zscene") {
                ++i;
                sceneName = args[i].CString();
            } else if (arg == "-zscene-idx") {
                sceneIdxGiven = true;
                ++i;
                sceneIdx = std::stoull(args[i].CString());
            }
            ++i;
        }
        if (sceneIdxGiven) {
            this->sceneIdx = sceneIdx;
        } else {
            auto it = this->sceneNameToIdx.find(sceneName);
            if (it != this->sceneNameToIdx.end()) {
                this->sceneIdx = it->second;
            } else {
                URHO3D_LOGERROR("Scene name not found.");
                this->engine_->Exit();
            }
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
        AppleButtonsAndComponent(Context* context) : Component(context) {}
        void Init(Main *main, bool simultaneous = false) {
            this->nButtonsHit = 0;
            this->active = true;
            this->main = main;
            this->simultaneous = simultaneous;
        }
        void AddChildButton(Node *node) {
            this->buttonsHit[node] = false;
            this->SubscribeToEvent(node, E_NODEBEGINCONTACT2D, URHO3D_HANDLER(AppleButtonsAndComponent, HandleChildBeginContact));
            if (simultaneous) {
                this->SubscribeToEvent(node, E_NODEENDCONTACT2D, URHO3D_HANDLER(AppleButtonsAndComponent, HandleChildEndContact));
            }
        }
    private:
        Main *main;
        std::map<Node*,bool> buttonsHit;
        decltype(buttonsHit)::size_type nButtonsHit;
        bool active, simultaneous;
        void HandleAppleEaten(StringHash eventType, VariantMap& eventData) {
            auto appleNode = (static_cast<Node*>(this->GetEventSender()));
            this->UnsubscribeFromEvent(appleNode, "Consumed");
            this->nButtonsHit = 0;
            this->active = true;
            for (auto &entry : this->buttonsHit) {
                Main::SetSprite(entry.first, this->main->resourceCache->GetResource<Sprite2D>("./button-finger.png"));
                entry.second = false;
            }
        }
        void HandleChildBeginContact(StringHash eventType, VariantMap& eventData) {
            if (this->simultaneous) {
                // Stop the player to make it easier for both to sync.
                // Ideally, this should be done with the restitution coefficient, but Box2D uses max(r1,r2)...
                // http://box2d.org/forum/viewtopic.php?t=659
                using namespace NodeBeginContact2D;
                auto otherNode = static_cast<Node*>(eventData[P_OTHERNODE].GetPtr());
                otherNode->GetComponent<RigidBody2D>()->SetLinearVelocity(Vector2::ZERO);
            }
            auto buttonNode = (static_cast<Node*>(this->GetEventSender()));
            auto &hit = this->buttonsHit.at(buttonNode);
            if (!hit) {
                this->nButtonsHit++;
                hit = true;
                Main::SetSprite(buttonNode, this->main->resourceCache->GetResource<Sprite2D>("./thumb-up.png"));
            }
            if (this->active && this->nButtonsHit == this->buttonsHit.size()) {
                Node *apple;
                this->main->CreateRandomAppleNode(apple, false);
                this->SubscribeToEvent(apple, "Consumed", URHO3D_HANDLER(AppleButtonsAndComponent, HandleAppleEaten));
                this->active = false;
            }
        }
        void HandleChildEndContact(StringHash eventType, VariantMap& eventData) {
            auto buttonNode = (static_cast<Node*>(this->GetEventSender()));
            auto &hit = this->buttonsHit.at(buttonNode);
            if (hit) {
                this->nButtonsHit--;
                hit = false;
                Main::SetSprite(buttonNode, this->main->resourceCache->GetResource<Sprite2D>("./button-finger.png"));
            }
        }
    };

    class HumanActorComponent : public Component {
        URHO3D_OBJECT(HumanActorComponent, Component);
    public:
        HumanActorComponent(Context* context) : Component(context) {
            this->SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(HumanActorComponent, HandleUpdate));
        }
        void Init() {
            this->forward = KEY_W;
            this->back = KEY_S;
            this->left = KEY_A;
            this->right = KEY_D;
            this->turnLeft = KEY_Q;
            this->turnRight = KEY_E;
        }
        void Init2() {
            this->forward = KEY_I;
            this->back = KEY_K;
            this->left = KEY_J;
            this->right = KEY_L;
            this->turnLeft = KEY_U;
            this->turnRight = KEY_O;
        }
    private:
        int forward, back, left, right, turnLeft, turnRight;
        void HandleUpdate(StringHash eventType, VariantMap& eventData) {
            auto playerBody = this->node_->GetComponent<RigidBody2D>();
            auto playerMass = Main::playerDensity * Main::playerRadius;
            auto input = this->GetSubsystem<Input>();
            auto playerForwardDirection = Vector2::UP;
            Main::Rotate2D(playerForwardDirection, this->node_->GetRotation2D());

            // Linear movement
            {
                auto forceMagnitude = 500.0f * playerMass;
                if (input->GetKeyDown(this->back)) {
                    Vector2 direction = playerForwardDirection;
                    Main::Rotate2D(direction, 180.0f);
                    playerBody->ApplyForceToCenter(direction * forceMagnitude, true);
                }
                if (input->GetKeyDown(this->forward)) {
                    Vector2 direction = playerForwardDirection;
                    playerBody->ApplyForceToCenter(direction * forceMagnitude, true);
                }
                if (input->GetKeyDown(this->left)) {
                    Vector2 direction = playerForwardDirection;
                    Main::Rotate2D(direction, 90.0f);
                    playerBody->ApplyForceToCenter(direction * forceMagnitude, true);
                }
                if (input->GetKeyDown(this->right)) {
                    Vector2 direction = playerForwardDirection;
                    Main::Rotate2D(direction, -90.0f);
                    playerBody->ApplyForceToCenter(direction * forceMagnitude, true);
                }
            }

            // Rotate
            auto torqueMagnitude = 20.0f * playerMass;
            if (input->GetKeyDown(this->turnLeft)) {
                playerBody->ApplyTorque(torqueMagnitude, true);
            }
            if (input->GetKeyDown(this->turnRight)) {
                playerBody->ApplyTorque(-torqueMagnitude, true);
            }
        }
    };

    class PlayerComponent : public Component {
        URHO3D_OBJECT(PlayerComponent, Component);
    public:
        PlayerComponent(Context* context) : Component(context) {}
        void Init(Main *main) {
            this->score = 0.0f;
            this->main = main;
            this->SubscribeToEvent(this->node_, E_NODEBEGINCONTACT2D, URHO3D_HANDLER(PlayerComponent, HandleNodeBeginContact2D));
        }
        float GetScore() { return this->score; }
        void IncrementScore(float inc) { this->score += inc; }
    private:
        Main *main;
        float score;
        void HandleNodeBeginContact2D(StringHash eventType, VariantMap& eventData) {
            using namespace NodeBeginContact2D;
            auto otherNode = static_cast<Node*>(eventData[P_OTHERNODE].GetPtr());
            {
                auto variant = otherNode->GetVar("TouchScoreChange");
                if (variant != Variant::EMPTY) {
                    this->IncrementScore(variant.GetFloat());
                }
            }
            {
                auto variant = otherNode->GetVar("IsConsumable");
                if (variant != Variant::EMPTY && variant.GetBool()) {
                    VariantMap eventData;
                    eventData["SENDER"] = this;
                    otherNode->SendEvent("Consumed", eventData);
                    if (otherNode->GetVar(RESPAWN).GetBool()) {
                        this->main->MoveToRandomEmptySpace(otherNode);
                        auto body = otherNode->GetComponent<RigidBody2D>();
                        body->SetLinearVelocity(Vector2::ZERO);
                        body->SetAngularVelocity(0.0f);
                    } else {
                        otherNode->Remove();
                    }
                }
            }
        }
    };

    class RottenAppleSpawnerComponent : public Component {
        URHO3D_OBJECT(RottenAppleSpawnerComponent, Component);
    public:
        RottenAppleSpawnerComponent(Context* context) : Component(context) {}
        void Init(Main *main) {
            this->main = main;
            this->SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(RottenAppleSpawnerComponent, HandleUpdate));
        }
    private:
        Main *main;
        void HandleUpdate(StringHash eventType, VariantMap& eventData) {
            auto node = this->GetScene()->CreateChild("RottenApple");
            // TODO for efficiency, cache the shape on constructor so we don't have to create the apples every time.
            // And then possibly monitor only node start contact and node end contact instead of doing this on update.
            this->main->CreateRottenAppleNode(node, false);
            node->SetPosition2D(this->node_->GetPosition2D());
            if (this->main->AabbCount(node->GetDerivedComponent<CollisionShape2D>()) > 1) {
                node->Remove();
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
                    "no-pain",
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
                    "competition",
                    "collaboration",
                    "collabotition",
                    "basketball",
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

    static void SetSprite(Node *node, Sprite2D *sprite, int layer = 0) {
        auto position = node->GetPosition2D();
        auto rotation = node->GetRotation();
        node->SetRotation(Quaternion(0.0f));
        auto b2Aabb = node->GetDerivedComponent<CollisionShape2D>()->GetFixture()->GetAABB(0);
        auto lowerBound = Vector2(b2Aabb.lowerBound.x, b2Aabb.lowerBound.y);
        auto upperBound = Vector2(b2Aabb.upperBound.x, b2Aabb.upperBound.y);
        PODVector<RigidBody2D*> rigidBodies;
        auto aabb = Rect(lowerBound, upperBound);
        auto staticSprite = node->GetOrCreateComponent<StaticSprite2D>();
        staticSprite->SetSprite(sprite);
        staticSprite->SetDrawRect(aabb - Rect(position, position));
        staticSprite->SetLayer(layer);
        node->SetRotation(rotation);
    }

    Node *playerNode;
    Text *text;
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
        // TODO: make player not lose speed when hitting the apple.
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

    void CreateCornerBouncers() {
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
    }

    void CreateGoldenAppleNode(Node *&node, bool respawn = true) {
        this->CreateAppleNodeBase(node, respawn);
        node->SetVar("TouchScoreChange", 5.0f);
        Main::SetSprite(node, this->resourceCache->GetResource<Sprite2D>("./shiny-apple-yellow.png"));
    }

    void CreatePlayerNode(Node *&node, bool attachCamera = true) {
        node = this->scene->CreateChild("Player");
        auto playerComponent = node->CreateComponent<PlayerComponent>();
        playerComponent->Init(this);
        auto humanActorComponent = node->CreateComponent<HumanActorComponent>();
        humanActorComponent->Init();
        auto body = node->CreateComponent<RigidBody2D>();
        body->SetBodyType(BT_DYNAMIC);
        body->SetLinearDamping(4.0);
        body->SetAngularDamping(4.0);
        body->SetBullet(true);
        auto shape = node->CreateComponent<CollisionCircle2D>();
        shape->SetDensity(Main::playerDensity);
        shape->SetFriction(0.0f);
        shape->SetRadius(Main::playerRadius);
        shape->SetRestitution(Main::playerRestitution);
        if (attachCamera) {
            this->CreateCamera(node, 20.0f * playerRadius);
        }
        Main::SetSprite(node, this->resourceCache->GetResource<Sprite2D>("./baby-face.png"));
    }

    void CreatePlayerNode(const Vector2& position, float rotation = 0.0f) {
        auto &node = this->playerNode;
        this->CreatePlayerNode(node);
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

    void CreateRandomPlayerNode(bool singlePlayer = true) {
        auto &node = this->playerNode;
        this->CreatePlayerNode(node, singlePlayer);
        this->MoveToRandomEmptySpace(node, singlePlayer);
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

    void CreateRottenAppleNode(Node *&node, bool respawn = true) {
        this->CreateAppleNodeBase(node, respawn);
        node->SetVar("TouchScoreChange", -1.0f);
        Main::SetSprite(node, this->resourceCache->GetResource<Sprite2D>("./shiny-apple-brown.png"));
    }

    void CreateWallNode(Node *&node, float length) {
        node = this->scene->CreateChild("Wall");
        node->CreateComponent<RigidBody2D>();
        auto shape = node->CreateComponent<CollisionBox2D>();
        shape->SetSize(Vector2(length, this->wallWidth));
        shape->SetRestitution(0.0);
        Main::SetSprite(node, this->resourceCache->GetResource<Sprite2D>("./gray.png"));
    }

    void CreateWallNodes() {
        Node *bottomWallNode;
        this->CreateWallNode(bottomWallNode, this->GetWindowWidth());
        bottomWallNode->SetName("BottomWall");
        bottomWallNode->SetPosition2D(Vector2(this->GetWindowWidth() / 2.0, this->wallWidth / 2.0f));
        {
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
        auto playerForwardDirection = Vector2::UP;
        Main::Rotate2D(playerForwardDirection, playerRotation);

        // Camera sensor
        std::vector<PhysicsRaycastResult2D> raycastResults;
        auto nrays = 8u;
        auto angleStep = -360.0f / nrays;
        for (auto i = 0u; i < nrays; ++i) {
            auto angle = i * angleStep;
            auto direction = playerForwardDirection;
            Main::Rotate2D(direction, angle);
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

        contactDataMap.clear();

        // Update score
        std::stringstream ss;
        if (false) {
            auto pos = this->playerNode->GetPosition2D();
            ss << "x,y: " << std::fixed << std::setprecision(1) << pos.x_ << ", " << pos.y_ << std::endl;
            auto vel = this->playerNode->GetComponent<RigidBody2D>()->GetLinearVelocity();
            ss << "vx,vy: " << std::fixed << std::setprecision(1) << vel.x_ << ", " << vel.y_ << std::endl;
        }
        ss << "Score: " << std::fixed << std::setprecision(0) << this->playerNode->GetComponent<PlayerComponent>()->GetScore();
        this->text->SetText(ss.str().c_str());
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

    void MoveToRandomEmptySpace(Node *node, bool randomRotation = true) {
        Rect bounds(0.0f, this->GetWindowHeight(), this->GetWindowWidth(), 0.0f);
        this->MoveToRandomEmptySpace(node, bounds, randomRotation);
    }

    // TODO: this uses the orthogonal AABB box. So a long diagonal object blocks the entire scene.
    void MoveToRandomEmptySpace(Node *node, const Rect& bounds, bool randomRotation = true) {
        do {
            node->SetPosition(Vector2(
                bounds.Left()   + (Random() * (bounds.Right() - bounds.Left())),
                bounds.Bottom() + (Random() * (bounds.Top()   - bounds.Bottom()))
            ));
            if (randomRotation) {
                node->SetRotation(Quaternion(Random() * 360.0f));
            }
        } while (this->AabbCount(node->GetDerivedComponent<CollisionShape2D>()) > 1);
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
