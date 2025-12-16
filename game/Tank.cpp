//
// Created by Pablo Gonzalez Poblette on 05/10/25.
//
#include "tank.h"

#include "ammoBox.h"
#include "utils.h"
#include "collision_manager.h"
#include "healthKit.h"

Tank::Tank(std::string colour)
{
	colorString = colour;
	// Load textures.
	// FIXME: loadFromFile returns a bool if texture was loaded successfully. We should use it to check for errors.

	if (!bodyTexture.loadFromFile("Assets/" + colour + "Tank.png"))
	{
		Utils::printMsg("Error loading texture from file",error);
	}
	if (!barrelTexture.loadFromFile("Assets/" + colour + "Barrel.png"))
	{
		Utils::printMsg("Error loading texture from file",error);
	}

	// Apply tetxures to sprites.
	body.setTexture(bodyTexture);
	barrel.setTexture(barrelTexture);

	// Reset texture rectangle. Applying new texture does not automatically apply it's size to sprite.
	body.setTextureRect(sf::IntRect({ 0, 0 }, static_cast<sf::Vector2i>(bodyTexture.getSize())));
	barrel.setTextureRect(sf::IntRect({ 0, 0 }, static_cast<sf::Vector2i>(barrelTexture.getSize())));

	body.setOrigin(static_cast<sf::Vector2f>(body.getTextureRect().getCenter()));
	barrel.setOrigin({ 6, 2 });

	// With the correct offset on the barrel, we can just set barrel position = body position.
	body.setPosition(position);
	barrel.setPosition(body.getPosition());

	body.setRotation(bodyRotation);
	barrel.setRotation(barrelRotation);
}

void Tank::Update(const float dt, const CollisionManager& collisionManager)
{
	if (!IsAlive())
		return;

	// Update rotation angle
	if (isMoving.left)
		bodyRotation -= sf::degrees(rotationSpeed * dt);
	else if (isMoving.right)
		bodyRotation += sf::degrees(rotationSpeed * dt);


	if (isAiming.left)
		barrelRotation -= sf::degrees(barrelSpeed * dt);
	else if (isAiming.right)
		barrelRotation += sf::degrees(barrelSpeed * dt);

	// Calculate direction vector from angle of rotation, based on the labs
	sf::Vector2f body_direction = {
		std::cos((bodyRotation - sf::degrees(90)).asRadians()),
		std::sin((bodyRotation - sf::degrees(90)).asRadians())
	};

	// Update position
	if (isMoving.forward)
		position -= body_direction * movementSpeed * dt;
	else if (isMoving.backward)
		position += body_direction * movementSpeed * dt;

	body.setRotation(bodyRotation);
	barrel.setRotation(barrelRotation);
	body.setPosition(position);
	barrel.setPosition(position);

	// Check for collisions after moving

	const sf::FloatRect bounds = GetBounds();

	if (sf::Vector2f pushback; collisionManager.CheckCollision(bounds, pushback))
	{
		// Move if collision detected
		position += pushback;

		body.setPosition(position);
		barrel.setPosition(position);
	}
}

sf::FloatRect Tank::GetBounds() const
{
	return body.getGlobalBounds();
}

void Tank::Shoot()
{
	if (ammo <= 0 && !IsAlive())
		return;

	// Calculate bullet spawn position at the end of the barrel
	float tipRotation = (barrelRotation + sf::degrees(90)).asRadians();

	// This is to get the rotation of the barrel so we now the direction of the tip
	sf::Vector2f barrelTip = position + sf::Vector2f{
		std::cos(tipRotation) * barrelLength,
		std::sin(tipRotation) * barrelLength
	};

	bullets.push_back(std::make_unique<bullet>(barrelTip, barrelRotation));
}

void Tank::UpdateBullets(float dt, CollisionManager& collisionManager)
{
	for (auto i = bullets.begin(); i != bullets.end();)
	{
		(*i)->Update(dt, collisionManager);

		// Remove bullet if  no longer active
		if (!(*i)->IsActive())
		{
			i = bullets.erase(i);
		}
		else
		{
			++i;
		}
	}
}

const void Tank::Render(sf::RenderWindow &window) {
	window.draw(body);
	window.draw(barrel);
}

void Tank::RenderBullets(sf::RenderWindow& window)
{
	for (auto& bullet : bullets)
	{
		bullet->Render(window);
	}
}

bool Tank::CheckPickupCollision(pickUp* pickup)
{
	if (!pickup || !pickup->IsActive())
		return false;

	if (pickup->CheckCollision(GetBounds()))
	{
		// Determine what type of pickup this is and handle accordingly
		if (auto* ammoBox = dynamic_cast<::ammoBox*>(pickup))
		{
			AddAmmo(ammoBox->GetAmmoAmount());
			ammoBox->Respawn();

			return true;
		}
		else if (auto* healthKit = dynamic_cast<::healthKit*>(pickup))
		{
			AddHealth(healthKit->GetHealAmount());
			healthKit->Respawn();

			return true;
		}
	}

	return false;
}

void Tank::TakeDamage(const int damage)
{
	health -= damage;
	if (health < 0)
		health = 0;

	Utils::printMsg("Tank took " + std::to_string(damage) + " damage. Health: " +
				   std::to_string(health), warning);

	if (health == 0)
	{
		Utils::printMsg("Tank Died =(", error);
		body.setColor(sf::Color(255, 0, 0));
		barrel.setColor(sf::Color(255, 0, 0));
	}
}


void Tank::AddAmmo(const int amount)
{
	if (ammo >= MAX_AMMO)
		return;

	int oldAmmo = ammo;
	ammo += amount;

	if (ammo > MAX_AMMO)
		ammo = MAX_AMMO;
}

void Tank::AddHealth(const int amount)
{
	if (health >= MAX_HEALTH)
		return;

	health += amount;

	if (health > MAX_HEALTH)
		health = MAX_HEALTH;
}


int Tank::getHealth()
{
	return health;
}

int Tank::getAmmo()
{
	return ammo;
}

int Tank::getMaxHealth()
{
	return MAX_HEALTH;
}

int Tank::getMaxAmmo()
{
	return MAX_AMMO;
}

void Tank::DecreaseAmmo(int amount)
{
	if (ammo >= 1)
	{
		ammo -= amount;
	}
}

void Tank::Reset()
{
	health = MAX_HEALTH;
	ammo = MAX_AMMO;

	body.setColor(sf::Color(255, 255, 255));
	barrel.setColor(sf::Color(255, 255, 255));

	bullets.clear();
}