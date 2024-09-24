/*
** EPITECH PROJECT, 2024
** R-Type
** File description:
** a_entity
*/

#pragma once

#include <memory>
#include <vector>

#include "error_handling.hpp"
#include "i_entity.hpp"

class AEntity : public IEntity {
    public:
        virtual void addComponent(std::shared_ptr<Component> component) override { components.push_back(component); }

        template <typename T>
        std::shared_ptr<T> getComponent();

    protected:
        std::vector<std::shared_ptr<Component>> components;
};

template <typename T>
std::shared_ptr<T> AEntity::getComponent() {
    std::shared_ptr<T> comp;

	for (auto &component : components) {
        comp = std::dynamic_pointer_cast<T>(component);
		if (comp != nullptr)
			return comp;
	}
	throw componentNotFound();
}