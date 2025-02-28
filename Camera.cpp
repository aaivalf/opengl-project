//
//  Camera.cpp
//  Lab5
//
//  Created by CGIS on 28/10/2016.
//  Copyright © 2016 CGIS. All rights reserved.
//

#include "Camera.hpp"

namespace gps {
    
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget)
    {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
    }
    
    glm::mat4 Camera::getViewMatrix()
    {
        return glm::lookAt(cameraPosition, cameraPosition + cameraDirection , glm::vec3(0.0f, 1.0f, 0.0f));
    }

	glm::vec3 Camera::getCameraTarget()
	{
		return cameraTarget;
	}

    glm::vec3 Camera::getCameraDirection()
    {
        return cameraDirection;
    }

    glm::vec3 Camera::getCameraPosition()
    {
        return cameraPosition;
    }
    
    void Camera::move(MOVE_DIRECTION direction, float speed)
    {
        switch (direction) {
            case MOVE_FORWARD:
                cameraPosition += cameraDirection * speed;
                break;
                
            case MOVE_BACKWARD:
                cameraPosition -= cameraDirection * speed;
                break;
                
            case MOVE_RIGHT:
                cameraPosition += cameraRightDirection * speed;
                break;
                
            case MOVE_LEFT:
                cameraPosition -= cameraRightDirection * speed;
                break;
        }
    }
    
    void Camera::rotate(float pitch, float yaw)
    {
        glm::mat4 rotation = glm::mat4(1.0f);

        rotation = glm::rotate(rotation, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
        rotation = glm::rotate(rotation, pitch, cameraRightDirection);

        glm::vec4 cameraPoint = glm::vec4(cameraDirection, 1.0f);
        cameraPoint = rotation * cameraPoint;
        glm::normalize(cameraPoint);

        cameraDirection = (glm::vec3)cameraPoint;
        cameraRightDirection = glm::cross(cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    void Camera::moveFreely(glm::vec3 position, glm::vec3 direction)
    {
        cameraPosition = position;
        cameraDirection = direction;
    }

}
