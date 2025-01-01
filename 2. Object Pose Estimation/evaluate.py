import os
import json
import numpy as np

best_acc = 1000
acc_id = 0
best_see = 0
see_id = 0
best_all = 1000
all_id = 0

for j in range(19, 20):
    see = 0
    acc = 0
    all = 0
    count = 0

    if j < 10:
        file_path = f"big_alt/handle_new/val/net_epoch_0{j}"
    else:
        file_path = f"big_alt/handle_new/val/net_epoch_{j}"
            
    for file_name in os.listdir(file_path):
        if file_name.endswith(".json"):
            files = os.path.join(file_path, file_name)
            with open(files, 'r', encoding='utf-8') as file:
                g = json.load(file)
                for obj in g["objects"]:
                    if "class" in obj and obj["class"] == "handle":
                        see +=1
                        break
    
    for i in range(1, 1262):
        projected_cuboid_keypoints = []
        projected_cuboid_keypoints_p = []
        if j < 10:
            file_path = f"big_alt/handle_new/evaluate/net_epoch_0{j}/{i}.json"
        else:
            file_path = f"big_alt/handle_new/evaluate/net_epoch_{j}/{i}.json"
            
        with open(file_path, 'r', encoding='utf-8') as file:
            p = json.load(file)
            for obj_p in p["objects"]:
                if "class" in obj_p and obj_p["class"] == "handle":
                    count += 1
                    projected_cuboid_keypoints_p = obj_p["projected_cuboid"]
                    
                    with open(f"evaluate/{i}.json", 'r', encoding='utf-8') as ground:
                        g = json.load(ground)
                        for obj in g["objects"]:
                            if "label" in obj and obj["label"] == "handle":
                                if "projected_cuboid" in obj:
                                    projected_cuboid_keypoints = [obj["projected_cuboid"][8], obj["projected_cuboid"][4], obj["projected_cuboid"][2], obj["projected_cuboid"][6], obj["projected_cuboid"][7], obj["projected_cuboid"][3], obj["projected_cuboid"][1], obj["projected_cuboid"][5], obj["projected_cuboid"][0]]
                                elif "cuboid_keypoints_projected" in obj:
                                    projected_cuboid_keypoints = [obj["cuboid_keypoints_projected"][8], obj["cuboid_keypoints_projected"][4], obj["cuboid_keypoints_projected"][2], obj["cuboid_keypoints_projected"][6], obj["cuboid_keypoints_projected"][7], obj["cuboid_keypoints_projected"][3], obj["cuboid_keypoints_projected"][1], obj["cuboid_keypoints_projected"][5], obj["cuboid_keypoints_projected"][0]]
                            elif "class" in obj and obj["class"] == "handle":
                                projected_cuboid_keypoints = [obj["projected_cuboid"][1], obj["projected_cuboid"][0], obj["projected_cuboid"][3], obj["projected_cuboid"][2], obj["projected_cuboid"][5], obj["projected_cuboid"][4], obj["projected_cuboid"][7], obj["projected_cuboid"][6], obj["projected_cuboid"][8]]             


                    squared_diff = 0.0
                    for x in range(9):
                        squared_diff += np.sqrt((projected_cuboid_keypoints_p[x][0]-projected_cuboid_keypoints[x][0])**2 + (projected_cuboid_keypoints_p[x][1]-projected_cuboid_keypoints[x][1])**2)
                        
                    acc += squared_diff/9
        
    see = see/1261
    
    if see > best_see:
        best_see = see
        see_id = j

    if count != 0:
        acc = acc/count
    else:
        acc = 100
    
    if acc < best_acc:
        best_acc = acc
        acc_id = j
        
    all = acc / see
    if all < best_all:
        best_all = all
        all_id = j
    
    print(f"{j}, {see}, {acc}, {all}")
        
print(f"best see, {see_id}, {best_see}")
print(f"best acc, {acc_id}, {best_acc}")
print(f"best all, {all_id}, {best_all}")