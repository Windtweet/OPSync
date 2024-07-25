import torch
import torchvision

# 检查CUDA是否可用
print("CUDA available: ", torch.cuda.is_available())

# 检查torchvision NMS是否可用
boxes = torch.tensor([[0, 0, 10, 10], [0, 0, 9, 9]], dtype=torch.float32).cuda()
scores = torch.tensor([0.9, 0.8], dtype=torch.float32).cuda()
nms_indices = torchvision.ops.nms(boxes, scores, 0.5)
print("NMS indices: ", nms_indices)
