#pragma once

#include "type_list.hpp"

namespace rpp {

// the base class of meta information containers
template <class Visitors>
struct MetaBase;

template <>
struct MetaBase<TypeList<>> {
    virtual const char *getName() = 0;
    virtual const decltype(typeid(void)) &getTypeInfo() = 0;
    virtual void *getPointer() = 0;

    // see: using MetaBase<TypeList<Args...>>::doVisit
    void doVisit() = delete;
};

template <class Visitor, class... Args>
struct MetaBase<TypeList<Visitor, Args...>>: public MetaBase<TypeList<Args...>> {
    using MetaBase<TypeList<Args...>>::doVisit;

    virtual typename Visitor::ReturnValue doVisit(Visitor &visitor) = 0;
};

// the implementation of MetaBase
template <class Visitors, class Accessor, class Base = MetaBase<Visitors>>
struct MetaImpl;

template <class Accessor, class Base>
struct MetaImpl<
    TypeList<>, Accessor, Base
>: public Base, Accessor {
    using Accessor::Accessor;
    using Base::doVisit;

    virtual const char *getName() override {
        return Accessor::getRealName();
    }

    virtual const decltype(typeid(void)) &getTypeInfo() override {
        return typeid(Accessor::access());
    }

    virtual void *getPointer() override {
        return &Accessor::access();
    }
};

template <class Visitor, class... Args, class Accessor, class Base>
struct MetaImpl<
    TypeList<Visitor, Args...>, Accessor, Base
>: public MetaImpl<TypeList<Args...>, Accessor, Base> {
    using MetaImpl<TypeList<Args...>, Accessor, Base>::MetaImpl;
    using MetaImpl<TypeList<Args...>, Accessor, Base>::doVisit;

    typename Visitor::ReturnValue doRealVisit(Visitor &visitor) {
        return visitor.visit(Accessor::access());
    }

    virtual typename Visitor::ReturnValue doVisit(Visitor &visitor) override {
        return doRealVisit(visitor);
    }
};

}

// ======== Usage example ========

#ifdef RPP_DEBUG

    #include <typeinfo>
    #include <iostream>
    #include <string>
    #include <vector>
    #include "visitor_chain.hpp"
    #include "accessor.hpp"

    namespace rpp {

        struct Visitor4: public VisitorBase<> {
            void visit(int &value) {
                std::cerr << "visitor4: int, " << value;
            }

            void visit(char &value) {
                std::cerr << "visitor4: char, " << value;
                value += 1;
            }

            void visit(bool &value) {
                std::cerr << "visitor4: bool, " << value;
            }
        };

        struct Visitor5: public VisitorBase<int> {
            template <class T>
            int visit(T &value) {
                std::cerr << "visitor5: " << typeid(T).name() << ", " << value;
                return 42;
            }
        };

        struct Accessor1: public AccessorLocal<int> {
            using AccessorLocal::AccessorLocal;

            const char *getRealName() {
                return "value1";
            }
        };

        struct Accessor2: public AccessorLocal<char> {
            using AccessorLocal::AccessorLocal;

            const char *getRealName() {
                return "value2";
            }
        };

        static char accessor_value = 'C';

        struct Accessor3: public AccessorStatic<char, accessor_value> {
            using AccessorStatic::AccessorStatic;

            const char *getRealName() {
                return "value3";
            }
        };

        struct Accessor4: public AccessorDynamic<char> {
            using AccessorDynamic::AccessorDynamic;

            const char *getRealName() {
                return "value4";
            }
        };

        RPP_VISITOR_REG(Visitor4)
        RPP_VISITOR_REG(Visitor5)
        RPP_VISITOR_COLLECT(VisitorAll3)

        static const int test1 = []() {
            MetaImpl<VisitorAll3, Accessor1> meta1{0};
            MetaImpl<VisitorAll3, Accessor2> meta2{'A'};
            MetaImpl<VisitorAll3, Accessor3> meta3;
            MetaImpl<VisitorAll3, Accessor4> meta4{accessor_value};
            std::vector<MetaBase<VisitorAll3> *> metalist{
                &meta1, &meta2, &meta3, &meta4
            };

            Visitor4 v4;
            Visitor5 v5;

            for (MetaBase<VisitorAll3> *meta: metalist) {
                std::cerr << meta->getName() << ": ";
                std::cerr << meta->getTypeInfo().name() << ", ";
                std::cerr << meta->getPointer() << " - ";
                meta->doVisit(v4);
                std::cerr << " - " << ", return " + std::to_string(meta->doVisit(v5));
                std::cerr << std::endl;
            }

            std::cerr << "sizeof(...):";
            std::cerr << " " << sizeof(Accessor1);
            std::cerr << " " << sizeof(Accessor2);
            std::cerr << " " << sizeof(Accessor3);
            std::cerr << " " << sizeof(Accessor4);
            std::cerr << " " << sizeof(meta1);
            std::cerr << " " << sizeof(meta2);
            std::cerr << " " << sizeof(meta3);
            std::cerr << " " << sizeof(meta4);
            // std::cerr << sizeof(meta12) << std::endl;
            std::cerr << std::endl;

            return 0;
        }();

    }

#endif