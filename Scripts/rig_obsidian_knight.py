"""
Ashen Ossuary — ObsidianKnight 자동 리깅 스크립트
에이전트 E 작성

실행 방법 A (Blender 설치된 경우 — 자동):
  blender --background --python scripts/rig_obsidian_knight.py

실행 방법 B (Blender GUI):
  1. Blender 열기
  2. 상단 탭 'Scripting' 클릭
  3. 이 파일 열기 (Open) 또는 내용 붙여넣기
  4. ▶ Run Script

완료 후 생성 파일:
  art/Character2/SK_GK_ObsidianKnight.fbx
  → UE Editor 에서 scripts/import_meshy_assets.py 재실행하면 자동 임포트

결과물:
  - UE5 Mannequin 호환 본 이름 (재타게팅 가능)
  - root / pelvis / spine_01~03 / neck_01 / head
  - clavicle_l/r / upperarm_l/r / lowerarm_l/r / hand_l/r
  - thigh_l/r / calf_l/r / foot_l/r / ball_l/r
"""

import bpy
import mathutils
import os
import sys

FBX_INPUT  = r"C:\UE\GK\art\Character2\SM_GK_ObsidianKnight.fbx"
FBX_OUTPUT = r"C:\UE\GK\art\Character2\SK_GK_ObsidianKnight.fbx"


# ── 씬 초기화 ──────────────────────────────────────────────────────────
def clear_scene():
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    for col in list(bpy.data.collections):
        bpy.data.collections.remove(col)


# ── FBX 임포트 및 메쉬 정리 ────────────────────────────────────────────
def import_and_merge_mesh():
    bpy.ops.import_scene.fbx(
        filepath=FBX_INPUT,
        use_custom_normals=True,
        use_image_search=False,
        automatic_bone_orientation=True,
        ignore_leaf_bones=True,
        force_connect_children=False,
    )

    mesh_objs = [o for o in bpy.context.scene.objects if o.type == 'MESH']
    if not mesh_objs:
        sys.exit("[GK Rig] 오류: FBX에서 메쉬를 찾을 수 없습니다.")

    # 기존 임포트된 아마추어 제거 (Meshy AI가 간혹 빈 bone 포함)
    for o in list(bpy.context.scene.objects):
        if o.type == 'ARMATURE':
            bpy.data.objects.remove(o, do_unlink=True)

    # 여러 메쉬 하나로 합치기
    bpy.ops.object.select_all(action='DESELECT')
    for obj in mesh_objs:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = mesh_objs[0]
    if len(mesh_objs) > 1:
        bpy.ops.object.join()

    mesh = bpy.context.view_layer.objects.active
    mesh.name = "SK_GK_ObsidianKnight"

    # Remove doubles / clean up
    bpy.context.view_layer.objects.active = mesh
    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.mesh.remove_doubles(threshold=0.001)
    bpy.ops.mesh.normals_make_consistent(inside=False)
    bpy.ops.object.mode_set(mode='OBJECT')

    return mesh


# ── 바운딩 박스 분석 ───────────────────────────────────────────────────
def get_bounds(obj):
    world_verts = [obj.matrix_world @ mathutils.Vector(v) for v in obj.bound_box]
    xs = [v.x for v in world_verts]
    ys = [v.y for v in world_verts]
    zs = [v.z for v in world_verts]
    h  = max(zs) - min(zs)
    return {
        'x_min': min(xs), 'x_max': max(xs),
        'y_min': min(ys), 'y_max': max(ys),
        'z_min': min(zs), 'z_max': max(zs),
        'cx': (min(xs) + max(xs)) / 2,
        'cy': (min(ys) + max(ys)) / 2,
        'h':  h,
        'z0': min(zs),
    }


# ── 아마추어 생성 (UE5 Mannequin 호환 본 이름) ───────────────────────
def create_ue5_armature(b):
    H  = b['h']
    z0 = b['z0']
    cx = b['cx']
    cy = b['cy']

    arm_data = bpy.data.armatures.new("SK_GK_ObsidianKnight_Armature")
    arm_obj  = bpy.data.objects.new("SK_GK_ObsidianKnight_Armature", arm_data)
    bpy.context.scene.collection.objects.link(arm_obj)
    bpy.context.view_layer.objects.active = arm_obj
    arm_obj.select_set(True)
    bpy.ops.object.mode_set(mode='EDIT')

    eb = arm_data.edit_bones

    def bone(name, head, tail, parent=None, connected=False):
        b_ = eb.new(name)
        b_.head = mathutils.Vector(head)
        b_.tail = mathutils.Vector(tail)
        if parent:
            b_.parent = eb[parent]
            b_.use_connect = connected
        return b_

    # 비율 상수 (표준 인체)
    FOOT_Z    = z0
    ANKLE_Z   = z0 + H * 0.07
    KNEE_Z    = z0 + H * 0.27
    HIP_Z     = z0 + H * 0.52
    S1_Z      = z0 + H * 0.58
    S2_Z      = z0 + H * 0.65
    S3_Z      = z0 + H * 0.73
    NECK_Z    = z0 + H * 0.84
    HEAD_Z    = z0 + H * 0.89
    HEADTOP_Z = z0 + H * 1.00

    SHD_X  = H * 0.14   # 어깨 좌우 거리
    ELBOW_X= H * 0.22
    HAND_X = H * 0.30
    ARM_Z  = z0 + H * 0.79
    ELBOW_Z= z0 + H * 0.61
    HAND_Z = z0 + H * 0.41

    HIP_X   = H * 0.07
    KNEE_X  = H * 0.05

    # Root / Pelvis
    bone("root",       (cx, cy, z0),     (cx, cy, z0 + H*0.04))
    bone("pelvis",     (cx, cy, HIP_Z),  (cx, cy, HIP_Z - H*0.05), "root")

    # Spine
    bone("spine_01",   (cx, cy, S1_Z),   (cx, cy, S2_Z),   "pelvis")
    bone("spine_02",   (cx, cy, S2_Z),   (cx, cy, S3_Z),   "spine_01")
    bone("spine_03",   (cx, cy, S3_Z),   (cx, cy, NECK_Z), "spine_02")

    # Neck / Head
    bone("neck_01",    (cx, cy, NECK_Z), (cx, cy, HEAD_Z),    "spine_03")
    bone("head",       (cx, cy, HEAD_Z), (cx, cy, HEADTOP_Z), "neck_01")

    # Clavicles
    bone("clavicle_l", (cx,           cy, ARM_Z), (cx + SHD_X,  cy, ARM_Z), "spine_03")
    bone("clavicle_r", (cx,           cy, ARM_Z), (cx - SHD_X,  cy, ARM_Z), "spine_03")

    # Arms — Left
    bone("upperarm_l", (cx + SHD_X,   cy, ARM_Z),   (cx + ELBOW_X, cy, ELBOW_Z), "clavicle_l")
    bone("lowerarm_l", (cx + ELBOW_X, cy, ELBOW_Z), (cx + HAND_X,  cy, HAND_Z),  "upperarm_l")
    bone("hand_l",     (cx + HAND_X,  cy, HAND_Z),  (cx + HAND_X + H*0.05, cy, HAND_Z - H*0.03), "lowerarm_l")

    # Arms — Right
    bone("upperarm_r", (cx - SHD_X,   cy, ARM_Z),   (cx - ELBOW_X, cy, ELBOW_Z), "clavicle_r")
    bone("lowerarm_r", (cx - ELBOW_X, cy, ELBOW_Z), (cx - HAND_X,  cy, HAND_Z),  "upperarm_r")
    bone("hand_r",     (cx - HAND_X,  cy, HAND_Z),  (cx - HAND_X - H*0.05, cy, HAND_Z - H*0.03), "lowerarm_r")

    # Legs — Left
    bone("thigh_l",    (cx + HIP_X,  cy, HIP_Z),   (cx + KNEE_X, cy, KNEE_Z),  "pelvis")
    bone("calf_l",     (cx + KNEE_X, cy, KNEE_Z),  (cx + KNEE_X, cy, ANKLE_Z), "thigh_l")
    bone("foot_l",     (cx + KNEE_X, cy, ANKLE_Z), (cx + KNEE_X, cy - H*0.09, ANKLE_Z), "calf_l")
    bone("ball_l",     (cx + KNEE_X, cy - H*0.09, ANKLE_Z), (cx + KNEE_X, cy - H*0.14, FOOT_Z), "foot_l")

    # Legs — Right
    bone("thigh_r",    (cx - HIP_X,  cy, HIP_Z),   (cx - KNEE_X, cy, KNEE_Z),  "pelvis")
    bone("calf_r",     (cx - KNEE_X, cy, KNEE_Z),  (cx - KNEE_X, cy, ANKLE_Z), "thigh_r")
    bone("foot_r",     (cx - KNEE_X, cy, ANKLE_Z), (cx - KNEE_X, cy - H*0.09, ANKLE_Z), "calf_r")
    bone("ball_r",     (cx - KNEE_X, cy - H*0.09, ANKLE_Z), (cx - KNEE_X, cy - H*0.14, FOOT_Z), "foot_r")

    bpy.ops.object.mode_set(mode='OBJECT')
    print(f"[GK Rig] 아마추어 생성 완료: 높이={H:.1f} 단위 ({H/100:.2f}m 추정)")
    return arm_obj


# ── 자동 웨이트 페인팅 ─────────────────────────────────────────────────
def auto_weight(mesh_obj, arm_obj):
    bpy.ops.object.select_all(action='DESELECT')
    mesh_obj.select_set(True)
    arm_obj.select_set(True)
    bpy.context.view_layer.objects.active = arm_obj
    # Envelope weights: 각 본의 영향 반경(envelope)에 따라 자동 할당
    bpy.ops.object.parent_set(type='ARMATURE_ENVELOPE')
    print("[GK Rig] 웨이트 페인팅 완료")


# ── FBX 익스포트 (UE5 호환 설정) ─────────────────────────────────────
def export_fbx(mesh_obj, arm_obj):
    bpy.ops.object.select_all(action='DESELECT')
    mesh_obj.select_set(True)
    arm_obj.select_set(True)
    bpy.context.view_layer.objects.active = arm_obj

    bpy.ops.export_scene.fbx(
        filepath           = FBX_OUTPUT,
        use_selection      = True,
        apply_unit_scale   = True,
        apply_scale_options= 'FBX_SCALE_ALL',
        bake_space_transform=True,
        object_types       = {'ARMATURE', 'MESH'},
        use_mesh_modifiers = True,
        mesh_smooth_type   = 'FACE',
        use_armature_deform_only=True,
        add_leaf_bones     = False,
        primary_bone_axis  = 'Y',
        secondary_bone_axis= 'X',
        axis_forward       = '-Z',
        axis_up            = 'Y',
        path_mode          = 'COPY',
        embed_textures     = False,
    )
    print(f"[GK Rig] ✓ 익스포트 완료: {FBX_OUTPUT}")


# ── 메인 ──────────────────────────────────────────────────────────────
def run():
    print("=" * 50)
    print("  Ashen Ossuary — ObsidianKnight 자동 리깅")
    print("=" * 50)

    print("[GK Rig] 1/5 씬 초기화...")
    clear_scene()

    print("[GK Rig] 2/5 FBX 임포트 및 메쉬 정리...")
    mesh = import_and_merge_mesh()

    print("[GK Rig] 3/5 바운딩 박스 분석...")
    bounds = get_bounds(mesh)
    print(f"[GK Rig]   높이: {bounds['h']:.1f} | 중심: ({bounds['cx']:.1f}, {bounds['cy']:.1f})")

    print("[GK Rig] 4/5 UE5 Mannequin 호환 아마추어 생성...")
    arm = create_ue5_armature(bounds)

    print("[GK Rig] 5/5 자동 웨이트 페인팅 + FBX 익스포트...")
    auto_weight(mesh, arm)
    export_fbx(mesh, arm)

    print("=" * 50)
    print("  완료!")
    print(f"  출력 파일: {FBX_OUTPUT}")
    print("  다음 단계: UE Editor > Tools > Execute Python Script")
    print("             > scripts/import_meshy_assets.py")
    print("=" * 50)


run()
