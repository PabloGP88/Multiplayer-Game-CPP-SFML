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
	bodyTexture.loadFromFile("Assets/" + colour + "Tank.png");
	barrelTexture.loadFromFile("Assets/" + colour + "Barrel.png");

	// Apply tetxures to sprites.
	body.setTexture(bodyTexture);
	barrel.setTexture(barrelTexture);

	// Reset texture rectangle. Applying new texture does not automatically apply it's size to sprite.
	body.setTextureRect(sf::IntRect({ 0, 0 }, (sf::Vector2i)bodyTexture.getSize()));
	barrel.setTextureRect(sf::IntRect({ 0, 0 }, (sf::Vector2i)barrelTexture.getSize()));

	// Set sprite origins. For bodym use the center of the texture. For barrel, hardcoded value.
	body.setOrigin((sf::Vector2f)body.getTextureRect().getCenter());
	barrel.setOrigin({ 6, 2 });

	// With the correct offset on the barrel, we can just set barrel position = body position.
	body.setPosition(position);
	barrel.setPosition(body.getPosition());

	// Set default barrel rotation to match body rotation.
	// FIXME: for actual tank game, we would have barrel rotated independently of the body.
	body.setRotation(bodyRotation);
	barrel.setRotation(barrelRotation);
}

void Tank::Update(float dt, CollisionManager& collisionManager)
{
	// Check if player is Alive
	if (!IsAlive())
		return;

	// Update rotation angle based on input.

	if (isMoving.left)
		bodyRotation -= sf::degrees(rotationSpeed * dt);
	else if (isMoving.right)
		bodyRotation += sf::degrees(rotationSpeed * dt);

	// Rotating the barrel based on input

	if (isAiming.left)
		barrelRotation -= sf::degrees(barrelSpeed * dt);
	else if (isAiming.right)
		barrelRotation += sf::degrees(barrelSpeed * dt);

	// Calculate direction vector from angle of rotation.
	sf::Vector2f body_direction = {
		std::cos((bodyRotation - sf::degrees(90)).asRadians()),
		std::sin((bodyRotation - sf::degrees(90)).asRadians())
	};

	// Update position based on input and direction.
	if (isMoving.forward)
		position -= body_direction * movementSpeed * dt;
	else if (isMoving.backward)
		position += body_direction * movementSpeed * dt;

	// Apply new rotation to tank body and barrel.
	body.setRotation(bodyRotation);
	barrel.setRotation(barrelRotation);

	// Apply new position to tank body and barrel.

	body.setPosition(position);
	barrel.setPosition(position);


	// Check for collisions after moving

	sf::Vector2f pushback;

	sf::FloatRect bounds = GetBounds();


	if (collisionManager.CheckCollision(bounds, pushback))
	{

		position += pushback; // Move if collision detected

		body.setPosition(position);
		barrel.setPosition(position);

		Utils::printMsg("Collision detected! Pushback: " + std::to_string(pushback.x) + ", " + std::to_string(pushback.y), debug);

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
	}; //Set the start position in the right pos of the tip


	// Create new bullet with barrel's rotation
	bullets.push_back(std::make_unique<bullet>(barrelTip, barrelRotation, 400.f));
	ammo -= 1;
}

void Tank::UpdateBullets(float dt, CollisionManager& collisionManager)
{
	// Update all bullets and remove inactive ones
	for (auto i = bullets.begin(); i != bullets.end();)
	{
		(*i)->Update(dt, collisionManager);
		Utils::printMsg("Bullet Updated in Local Tank", debug);

		// Remove bullet if it's no longer active
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

void Tank::CheckPickupCollision(pickUp* pickup)
{
	if (!pickup || !pickup->IsActive())
		return;

	if (pickup->CheckCollision(GetBounds()))
	{
		// Determine what type of pickup this is and handle accordingly
		if (auto* ammoBox = dynamic_cast<::ammoBox*>(pickup))
		{
			AddAmmo(ammoBox->GetAmmoAmount());
			ammoBox->OnPickup();
			ammoBox->Respawn();
		}
		else if (auto* healthKit = dynamic_cast<::healthKit*>(pickup))
		{
			AddHealth(healthKit->GetHealAmount());
			healthKit->OnPickup();
			healthKit->Respawn();
		}
	}
}

void Tank::TakeDamage(int damage)
{
	health -= damage;
	if (health < 0)
		health = 0;

	Utils::printMsg("Tank took " + std::to_string(damage) + " damage. Health: " +
				   std::to_string(health), warning);

	if (health == 0)
	{
		Utils::printMsg("Tank destroyed", error);
	}
}


void Tank::AddAmmo(int amount)
{
	if (ammo >= MAX_AMMO)
		return;

	int oldAmmo = ammo;
	ammo += amount;

	if (ammo > MAX_AMMO)
		ammo = MAX_AMMO;

	int actualAmount = ammo - oldAmmo;
	Utils::printMsg("Added " + std::to_string(actualAmount) + " ammo. Total: " +
				   std::to_string(ammo) + "/" + std::to_string(MAX_AMMO), success);
}

void Tank::AddHealth(int amount)
{
	if (health >= MAX_HEALTH)
		return;

	int oldHealth = health;
	health += amount;

	if (health > MAX_HEALTH)
		health = MAX_HEALTH;

	int actualAmount = health - oldHealth;
	Utils::printMsg("Healed " + std::to_string(actualAmount) + " HP. Total: " +
				   std::to_string(health) + "/" + std::to_string(MAX_HEALTH), success);
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
	bullets.clear();
}