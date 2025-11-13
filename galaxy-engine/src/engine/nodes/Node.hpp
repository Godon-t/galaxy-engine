#pragma once

#include "engine/event/ActionEvent.hpp"

#include "engine/types/Math.hpp"
#include "visitors/NodeVisitor.hpp"

using namespace math;

class Root;

namespace Galaxy {
class Node {
private:
    friend class NodeVisitor;
    friend class Root;
    void update(double delta);
    std::string m_name;
    bool m_inRoot = false;

    static size_t s_rootID;
    bool m_paused;

    static std::unordered_map<size_t, std::weak_ptr<Node>> s_nodeByIds;

protected:
    Node* m_parent = nullptr;
    std::vector<std::shared_ptr<Node>> m_children; // Changement ici pour unique_ptr

public:
    static bool nodeExists(size_t id);
    static inline std::weak_ptr<Node> getNode(size_t id) { return s_nodeByIds[id]; }

    const size_t id;
    Node(std::string name = "Node")
        : m_name(name)
        , id(s_rootID++)
        , m_paused(false) {};
    Node(const Node&) = delete; // Interdit la copie
    Node(Node&&)      = default;

    virtual ~Node() = default;

    void setParent(Node* parent);

    Node* getParent() const;

    void addChild(std::shared_ptr<Node> child);
    void removeChild(Node* component);
    void clearChilds();
    std::vector<Node*> getChildren() const;
    size_t getChildCount() const;
    bool getInRoot() const { return m_inRoot; }

    std::string getName() { return m_name; }

    bool isLeaf() const
    {
        return m_children.empty();
    }

    void destroy(bool destroyByParent = false);

    void enterRoot();

    virtual void process(double delta) { }
    virtual void draw();

    virtual void updateTransformAndChilds(const mat4& matrix = mat4(1));
    virtual void forceUpdateTransformAndChilds(const mat4& matrix);

    void handleInput(const Event& event);
    virtual void handleInputFromTop(const Event& event) { }
    virtual void handleInputFromBot(const Event& event) { }

    virtual void accept(NodeVisitor& visitor);

    inline void activate() { m_paused = false; }
    inline void disable() { m_paused = true; }

protected:
    virtual void enteringRoot() { }
    virtual void enteredRoot() { }
};
}