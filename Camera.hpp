//
//  Camera.hpp
//
//  Created by CGIS on 28/10/2016.
//  Copyright © 2016 CGIS. All rights reserved.
//

#ifndef Camera_hpp
#define Camera_hpp

#include <stdio.h>
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

namespace gps {
    
    enum MOVE_DIRECTION {MOVE_FORWARD, MOVE_BACKWARD, MOVE_RIGHT, MOVE_LEFT};
    
    class Camera
    {
    public:
        Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget);
        glm::mat4 getViewMatrix();
		glm::vec3 getCameraTarget();
        glm::vec3 getCameraPosition();
        glm::vec3 getCameraDirection();
		void move(MOVE_DIRECTION direction, float speed);
        void rotate(float pitch, float yaw);
        void moveFreely(glm::vec3 position, glm::vec3 direction);
        
    private:
        glm::vec3 cameraPosition;
        glm::vec3 cameraTarget;
        glm::vec3 cameraDirection;
        glm::vec3 cameraRightDirection;
    };
    
}

#endif /* Camera_hpp */
