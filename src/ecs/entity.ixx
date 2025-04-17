module;

export module Entity;

// Unique identifier for all entities 
export {
    class Entity {
        unsigned int m_id;
        static unsigned int id_count;   // defaults to 0 (invalid), need to init 1

    public:

        Entity() {
            // ensure that each entity gets a unique ID
            m_id = id_count++; // assign and increment
        }

        Entity(unsigned int id) {
            m_id = id;
        }

        ~Entity() {
        }

        bool operator== (Entity& other) {
            return id() == other.id();
        }

        Entity static null() {
            return Entity(0);
        }

        operator unsigned int() const { return m_id; } // enables automatic casting to int

        unsigned int id() const { return m_id; }
    };

    unsigned int Entity::id_count = 1;
};


