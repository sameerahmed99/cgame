#include "camera.h"
#include "math.h"

Vec3 cam_world_to_screen_pos(Camera* _cam, Vec3 _pos){
  Vec3 screen = math_vec3_scale(_pos, _cam->ppu);

  return screen;
}

Vec3 cam_world_to_screen_vec(Camera* _cam, Vec3 _vec){
  Vec3 screen = math_vec3_scale(_vec, _cam->ppu);

  return screen;

}
