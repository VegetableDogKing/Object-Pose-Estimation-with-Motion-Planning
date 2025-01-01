# Debugging Tool to Visualize Synthetic Data Projected Points Accuracy

from PIL import Image
import json

from utils import Draw, loadimages

import argparse
import os


def visualize_projected_points(path_img, path_json, path_output, img_name, root):
    img = Image.open(path_img).convert("RGB")

    with open(path_json) as f:
        data_json = json.load(f)

    draw = Draw(img)

    for obj in data_json["objects"]:
        if obj["label"] == "handle":
            if obj["visibility"] > 0.6:
                if "cuboid_keypoints_projected" in obj:
                    keypoints = [obj["cuboid_keypoints_projected"][8], obj["cuboid_keypoints_projected"][4], obj["cuboid_keypoints_projected"][2], obj["cuboid_keypoints_projected"][6], obj["cuboid_keypoints_projected"][7], obj["cuboid_keypoints_projected"][3], obj["cuboid_keypoints_projected"][1], obj["cuboid_keypoints_projected"][5], obj["cuboid_keypoints_projected"][0]]
                    projected_cuboid_keypoints = [tuple(pair) for pair in keypoints]
                else:
                    keypoints = [obj["projected_cuboid"][8], obj["projected_cuboid"][4], obj["projected_cuboid"][2], obj["projected_cuboid"][6], obj["projected_cuboid"][7], obj["projected_cuboid"][3], obj["projected_cuboid"][1], obj["projected_cuboid"][5], obj["projected_cuboid"][0]]
                    projected_cuboid_keypoints = [tuple(pair) for pair in keypoints]
                draw.draw_cube(projected_cuboid_keypoints)

    path_output = os.path.join(
        path_output, img_path.replace(root, "").replace(img_name, "").lstrip("/")
    )
    os.makedirs(path_output, exist_ok=True)
    img.save(os.path.join(path_output, img_name))


if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "--outf",
        default="output/debug",
        help="Where to store the debug output images.",
    )
    parser.add_argument(
        "--data",
        required=True,
        help="Folder containing groundtruth and images.",
    )

    opt = parser.parse_args()

    imgs = sorted(loadimages(opt.data, extensions=["jpg", "png"]))

    for i, (img_path, img_name, json_path) in enumerate(imgs):
        img_rel_path = img_path.replace(opt.data, "")
        print(f"Debugging image {img_rel_path} ({i + 1} of {len(imgs)}) | Outputting to: {opt.outf + '/' + img_rel_path}")
        visualize_projected_points(img_path, json_path, opt.outf, img_name, opt.data)
