﻿#pragma once
#include <unordered_map>
#include "MMDPlugin/mmd_plugin.h"
#include <EffekseerRendererDX9.h>
#include "mmd/MMDExport.h"
#include <array>
#include <cassert>
#include <functional>

namespace efk
{
  struct PMDResource
  {
    PMDResource(int i);


    float triggerVal(int i) const;
    float autoPlayVal(int i) const;
    float frameVal(int i) const;
    float loopVal(int i) const;
    float triggerEraseVal(int i) const;
    float scaleUpVal(int i) const;
    float scaleDownVal(int i) const;
    float speedUpVal(int i) const;
    float speedDownVal(int i) const;
    float effectTestVal(int i) const;
    float stopRootVal(int i) const;


    D3DMATRIX playBone(int i) const;
    D3DMATRIX centerBone(int i) const;
    D3DMATRIX baseBone(int i) const;

  private:
    enum class MorphKind
    {
      trigger_morph,
      auto_play_morph,
      frame_morph,
      loop_morph,
      trigger_erase_morph,
      scale_up_morph,
      scale_down_morph,
      speed_up_morph,
      speed_down_morph,
      stop_root_morph,
      at_effect_test_morph,

      MORPH_RESOURCE_SIZE,
    };

    static constexpr int morph_resource_size = (int)MorphKind::MORPH_RESOURCE_SIZE;
    static constexpr std::array<std::array<const char*, 2>, morph_resource_size> morph_name_ = {
      std::array<const char*, 2>{ "trigger","トリガー" },
      { "auto play","オート再生" },
      { "frame","フレーム" },
      { "loop","ループ" },
      { "trigger erase","トリガー削除" },
      { "scale up","拡大" },
      { "scale down","縮小" },
      { "speed up","速度UP" },
      { "speed down","速度DOWN" },
      { "stop root","stop root" },
      { "@effect test","@エフェクトテスト" },
    };

    enum class BoneKind
    {
      play_bone,
      center_bone,
      base_bone,

      BONE_RESOURCE_SIZE,
    };

    static constexpr int bone_resource_size = (int)BoneKind::BONE_RESOURCE_SIZE;
    static constexpr std::array<std::array<const char*, 2>, bone_resource_size> bone_name_ = {
      std::array<const char*, 2>{ "play","再生" },
      { "center","センター" },
      { "base","ベース" },
    };

    std::array<int, bone_resource_size> bone_id_;
    std::array<int, morph_resource_size> morph_id_;


    static const char* getName(MorphKind k)
    {
      assert(morph_name_[static_cast<int>(k)][0]);
      assert(morph_name_[static_cast<int>(k)][1]);
      return morph_name_[static_cast<int>(k)][1];
    }

    static const char* getName(BoneKind k)
    {
      assert(bone_name_[static_cast<int>(k)][0]);
      assert(bone_name_[static_cast<int>(k)][1]);
      return bone_name_[static_cast<int>(k)][1];
    }

    float getMorph(int i, MorphKind k) const
    {
      return ExpGetPmdMorphValue(i, getID(k));
    }

    D3DMATRIX getBone(int i, BoneKind k) const
    {
      static D3DMATRIX indendity{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
      };
      int id = getID(k);
      return id == -1 ? indendity : ExpGetPmdBoneWorldMat(i, id);
    }

    int getID(MorphKind k) const { return morph_id_[static_cast<int>(k)]; }

    int getID(BoneKind k) const { return bone_id_[static_cast<int>(k)]; }
  };

  struct DeltaTime
  {
    void update()
    {
      pre_mmd_time_ += get() / 30.0f;
    }

    int get() const
    {
      float time = ExpGetFrameTime();
      return (time - pre_mmd_time_) * 30;
    }

  private:
    float pre_mmd_time_ = 0;
  };

  struct CommonEffect
  {
    virtual ~CommonEffect() = default;

    CommonEffect(Effekseer::Manager* manager, Effekseer::Effect* effect,
                 PMDResource* resource, DeltaTime* delta_time)
      :manager_(manager), effect_(effect), resource_(resource),
      delta_time_(delta_time)
    {
    }
    virtual void update(int i) = 0;

    virtual void updateHandle(int i, const std::function<void(Effekseer::Handle, float delta_time)>& update_func) = 0;

    virtual void draw() const = 0;
  protected:
    float getSpeed(int i) const { return 1.0f + resource_->speedUpVal(i) - resource_->speedDownVal(i); }

    Effekseer::Manager* manager_;
    Effekseer::Effect* effect_;
    PMDResource* resource_;
    DeltaTime* delta_time_;
    ;
  };

  struct TriggerTypeEffect : CommonEffect
  {
    TriggerTypeEffect(Effekseer::Manager* manager, Effekseer::Effect* effect, PMDResource* resource_, DeltaTime* delta_time)
      : CommonEffect(manager, effect, resource_, delta_time)
    {
    }

    void draw() const override;
    void push();
    void update(int i) override;

    void updateHandle(int i, const std::function<void(Effekseer::Handle, float)>& update_func) override;

  private:

    bool pre_triggerd_ = false;
    std::vector<Effekseer::Handle> new_effect_handle_;
    std::vector<Effekseer::Handle> effects_;

  };

  struct AutoPlayTypeEffect : CommonEffect
  {
    AutoPlayTypeEffect(Effekseer::Manager* manager, Effekseer::Effect* effect, PMDResource* resource_, DeltaTime* delta_time)
      : CommonEffect(manager, effect, resource_, delta_time)
    {
    }

    void ifCreate();
    void update(int i) override;
    void updateHandle(int i, const std::function<void(Effekseer::Handle, float)>& update_func_) override;
    void draw() const override;

  private:

    Effekseer::Handle handle_ = -1;
  };

  struct FrameTypeEffect : CommonEffect
  {
    FrameTypeEffect(Effekseer::Manager* manager, Effekseer::Effect* effect, PMDResource* resource_, DeltaTime* delta_time)
      : CommonEffect(manager, effect, resource_, delta_time)
    {
    }

    void ifCreate();
    void update(int i) override;
    void updateHandle(int i, const std::function<void(Effekseer::Handle, float)>& update_func_) override;
    void draw() const override;

  private:
    Effekseer::Handle handle_ = -1;
    float now_frame_ = 0;
  };

  struct MyEffect
  {
    MyEffect();
    MyEffect(Effekseer::Manager* manager, Effekseer::Effect* effect, PMDResource resource_);

    ~MyEffect();

    void draw(int i) const;

    void OnLostDevice() const;
    void OnResetDevice() const;


    void setMatrix(const D3DMATRIX& center, const D3DMATRIX& base);
    void setScale(float x, float y, float z);

    float getSpeed(int i) const;

    void update(int i);

    std::shared_ptr<PMDResource> resource_;

  private:

    void UpdateHandle(Effekseer::Handle handle, float delta_time);

    /* 共通 */

    Effekseer::Manager* manager_;
    Effekseer::Effect* effect_;
    Effekseer::Matrix43 base_matrix_;
    Effekseer::Matrix43 matrix_;
    Effekseer::Vector3D scale_;

    /* オート再生、フレーム */


    std::shared_ptr<DeltaTime> delta_time_;
    TriggerTypeEffect trigger_;
    AutoPlayTypeEffect auto_paly_;
    FrameTypeEffect frame_;
    /* 描画テスト */
    Effekseer::Handle effect_test_handle_;
  };

  class DistortingCallback : public EffekseerRenderer::DistortingCallback
  {
  public:
    DistortingCallback(::EffekseerRendererDX9::Renderer* renderer,
                       LPDIRECT3DDEVICE9 device, int texWidth, int texHeight);

    virtual ~DistortingCallback();

    virtual bool OnDistorting();
    void OnLostDevice();
    void OnResetDevice();
  private:

    bool use_distoring_ = true;
    ::EffekseerRendererDX9::Renderer* renderer = nullptr;
    LPDIRECT3DDEVICE9 device = nullptr;
    int width_, height_;
    LPDIRECT3DTEXTURE9 texture = nullptr;
  };

  struct D3D9DeviceEffekserr : public MMDPluginDLL2
  {
    D3D9DeviceEffekserr(IDirect3DDevice9* device);

    ~D3D9DeviceEffekserr();

    void UpdateCamera() const;
    void UpdateProjection() const;

    void HookAPI();
    void RestoreHook();

    void DrawIndexedPrimitive(D3DPRIMITIVETYPE, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) override;
    void BeginScene(THIS) override;
    void EndScene(THIS) override;

    void Reset(D3DPRESENT_PARAMETERS* pPresentationParameters) override;
    void PostReset(D3DPRESENT_PARAMETERS* pPresentationParameters, HRESULT& res) override;

  private:

    void SetDistorting();

    EffekseerRendererDX9::Renderer* renderer_;
    Effekseer::Manager* manager_;

    bool now_present_;
    std::unordered_map<std::wstring, int> effect_id_;
    // <ID, Effect>
    std::unordered_map<int, MyEffect> effect_;
    IDirect3DDevice9* device_;
    DistortingCallback* distorting_callback_ = nullptr;
  };
}
