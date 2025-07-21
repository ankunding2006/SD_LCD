from maix import camera, display, image, nn, app, uart
import time
import math

# --- 追踪和平滑相关参数 ---
# 用于存储追踪的对象
tracked_objects = []
# 指数移动平均法的平滑因子 (alpha)，值越大，新检测框权重越高，响应越快但平滑效果越弱
EMA_ALPHA = 0.5
# 匹配新旧检测框的最大像素距离
MAX_DISTANCE = 200
# 一个被追踪对象在被删除前可以“消失”的最大帧数
MAX_UNSEEN_FRAMES = 25000
# 允许标签不匹配的最大连续帧数，超过则更新标签
MAX_LABEL_MISMATCH_FRAMES = 100
# --------------------------

# --- 串口发送逻辑参数 ---
# 单个数字需要稳定出现的秒数
SINGLE_DIGIT_HOLD_SECONDS = 2.0
# 多个数字需要稳定出现的秒数
MULTI_DIGIT_HOLD_SECONDS = 0.8
# --------------------------

model_path = "/root/user_models/model_223174.mud"
# model_path = "/root/user_models/model_139433.mud" 如果想试试各个模型的性能就把模型的文件扔到我这个文件夹下面

# 训练个好模型把 这个v5 还画幅小
detector = nn.YOLOv5(model=model_path, dual_buff=True)

# 224*224 看不全 可能咱训练得用大画幅的 远一点 目前这个勉强能用而已
cam = camera.Camera(detector.input_width(), detector.input_height(), detector.input_format())
disp = display.Display()

# 串口初始化
device = "/dev/ttyS0"
serial = uart.UART(device, 115200)

# 用于存储当前识别的数字及其位置
current_numbers = []

# 用于串口发送的内部状态变量
candidate_str = None
candidate_start_time = 0
last_sent_value = None

# 跳过开头的30帧，等待摄像头图像稳定
print("Waiting for camera to stabilize...")
cam.skip_frames(30)
print("Camera ready.")


def update_tracked_objects(new_detections, tracked_list):
    """
    更新被追踪的对象列表，优化匹配逻辑并增加内部去重。
    """
    # 标记所有对象在本帧中尚未匹配
    for tracked in tracked_list:
        tracked['matched_in_frame'] = False
    for new_det in new_detections:
        new_det['matched_in_frame'] = False

    # --- 阶段 1: 优先匹配标签相同的对象 ---
    for new_det in new_detections:
        best_match = None
        min_dist = MAX_DISTANCE
        for tracked in tracked_list:
            # 仅在标签相同且尚未匹配时进行比较
            if new_det['label'] == tracked['label'] and not tracked.get('matched_in_frame', False):
                center_new = (new_det['x'] + new_det['w'] / 2, new_det['y'] + new_det['h'] / 2)
                center_tracked = (tracked['x'] + tracked['w'] / 2, tracked['y'] + tracked['h'] / 2)
                dist = math.sqrt((center_new[0] - center_tracked[0])**2 + (center_new[1] - center_tracked[1])**2)
                if dist < min_dist:
                    min_dist = dist
                    best_match = tracked
        
        if best_match:
            # 找到同类匹配，更新状态
            best_match['matched_in_frame'] = True
            new_det['matched_in_frame'] = True
            best_match['unseen_frames'] = 0
            best_match['label_mismatch_count'] = 0 # 重置不匹配计数
            # 平滑更新
            best_match['x'] = EMA_ALPHA * new_det['x'] + (1 - EMA_ALPHA) * best_match['x']
            best_match['y'] = EMA_ALPHA * new_det['y'] + (1 - EMA_ALPHA) * best_match['y']
            best_match['w'] = EMA_ALPHA * new_det['w'] + (1 - EMA_ALPHA) * best_match['w']
            best_match['h'] = EMA_ALPHA * new_det['h'] + (1 - EMA_ALPHA) * best_match['h']
            best_match['score'] = new_det['score']

    # --- 阶段 2: 为无法匹配同类的 “新” 检测，寻找最近的 “旧” 对象 (处理标签抖动) ---
    for new_det in new_detections:
        if new_det['matched_in_frame']:
            continue
        best_match = None
        min_dist = MAX_DISTANCE
        for tracked in tracked_list:
            if not tracked.get('matched_in_frame', False):
                center_new = (new_det['x'] + new_det['w'] / 2, new_det['y'] + new_det['h'] / 2)
                center_tracked = (tracked['x'] + tracked['w'] / 2, tracked['y'] + tracked['h'] / 2)
                dist = math.sqrt((center_new[0] - center_tracked[0])**2 + (center_new[1] - center_tracked[1])**2)
                if dist < min_dist:
                    min_dist = dist
                    best_match = tracked
        
        if best_match:
            # 找到了一个位置足够近的异类对象
            best_match['matched_in_frame'] = True
            new_det['matched_in_frame'] = True
            best_match['unseen_frames'] = 0
            best_match['label_mismatch_count'] += 1
            # 平滑更新位置
            best_match['x'] = EMA_ALPHA * new_det['x'] + (1 - EMA_ALPHA) * best_match['x']
            best_match['y'] = EMA_ALPHA * new_det['y'] + (1 - EMA_ALPHA) * best_match['y']
            best_match['w'] = EMA_ALPHA * new_det['w'] + (1 - EMA_ALPHA) * best_match['w']
            best_match['h'] = EMA_ALPHA * new_det['h'] + (1 - EMA_ALPHA) * best_match['h']
            best_match['score'] = new_det['score']
            # 如果不匹配次数超过阈值，则更新标签
            if best_match['label_mismatch_count'] > MAX_LABEL_MISMATCH_FRAMES:
                best_match['label'] = new_det['label']
                best_match['label_mismatch_count'] = 0

    # --- 阶段 3: 处理本轮结束后仍未匹配的对象 ---
    # 未匹配的新检测，是全新的对象
    for new_det in new_detections:
        if not new_det['matched_in_frame']:
            new_det['unseen_frames'] = 0
            new_det['label_mismatch_count'] = 0
            tracked_list.append(new_det)

    # 未匹配的旧追踪，增加其“消失”计数，并在太久未见时移除
    final_tracked_list = []
    for tracked in tracked_list:
        if not tracked['matched_in_frame']:
            tracked['unseen_frames'] += 1
        if tracked['unseen_frames'] < MAX_UNSEEN_FRAMES:
            final_tracked_list.append(tracked)

    # --- 阶段 4: 内部去重，确保最终列表中没有重复标签 ---
    # 这是防止在标签切换瞬间产生重复的关键一步
    unique_labels_seen = {}
    final_unique_list = []
    # 从右到左遍历，保留最新的（刚添加的或刚更新的）
    for tracked_obj in reversed(final_tracked_list):
        if tracked_obj['label'] not in unique_labels_seen:
            unique_labels_seen[tracked_obj['label']] = True
            final_unique_list.insert(0, tracked_obj)

    return final_unique_list


def is_valid_combination(s):
    """
    根据用户定义的规则，检查一个数字字符串是否为有效组合。
    """
    n = len(s)
    if n == 0 or n == 1:
        # 0个或1个数字总是有效的
        return True

    valid_set = {'5', '6', '7', '8'}

    if n == 2:
        # 规则1: 必须是 '34' 或 '43'
        if sorted(s) == ['3', '4']:
            return True
        # 规则2: 必须是 {5, 6, 7, 8} 中任意两个不同数字的组合
        if s[0] in valid_set and s[1] in valid_set:
            return True
        # 其他所有2位数组合都是无效的
        return False

    if n == 3:
        # 规则3: 必须是 {5, 6, 7, 8} 中任意三个不同数字的组合
        return all(char in valid_set for char in s)

    # 任何超过3个数字的组合都是无效的
    return False


def create_data_frame(data_str):
    """
    根据新的16进制协议创建数据帧。
    协议: 0x55 + byte1 + byte2 + 0x6B
    - 1个数字 'd1':       byte1=d1*16, byte2=0
    - 2个数字 'd1d2':     byte1=d1*16+d2, byte2=0
    - 3个数字 'd1d2d3':   byte1=d1*16+d2, byte2=d3*16
    """
    SOF = b'\x55'
    EOF = b'\x6B'

    byte1 = 0
    byte2 = 0
    
    digits = [int(d) for d in data_str]
    num_digits = len(digits)

    if num_digits == 1:
        d1 = digits[0]
        byte1 = d1 << 4
    elif num_digits == 2:
        d1, d2 = digits
        byte1 = (d1 << 4) | d2
    elif num_digits == 3:
        d1, d2, d3 = digits
        byte1 = (d1 << 4) | d2
        byte2 = d3 << 4
    
    # 组装帧
    frame = SOF + bytes([byte1]) + bytes([byte2]) + EOF
    return frame


def process_and_send_data(numbers):
    """
    处理并发送数据，所有有效组合都需要稳定一段时间后才发送。
    """
    global candidate_str, candidate_start_time, last_sent_value

    # 1. 预处理，得到干净、有效的当前字符串
    numbers.sort(key=lambda x: x['x'])
    raw_str = ''.join([num['label'] for num in numbers])
    
    if len(raw_str) > 1:
        deduped_str = raw_str[0]
        for i in range(1, len(raw_str)):
            if raw_str[i] != raw_str[i-1]:
                deduped_str += raw_str[i]
        current_str = deduped_str
    else:
        current_str = raw_str
        
    if not is_valid_combination(current_str):
        candidate_str = None # 如果组合无效，重置计时
        return

    # 2. 统一的状态机处理逻辑
    num_count = len(current_str)

    if num_count == 0:
        # --- 逻辑: 没有识别到任何数字 ---
        candidate_str = None
        if last_sent_value is not None and last_sent_value != "":
            last_sent_value = ""
        return

    # --- 逻辑: 识别到有效数字 ---
    if candidate_str != current_str:
        # 如果是新的候选者（或从无到有，或内容变化），重置计时器
        candidate_str = current_str
        candidate_start_time = time.time()
    else:
        # 如果是同一个候选者，检查是否满足发送时间
        hold_time = SINGLE_DIGIT_HOLD_SECONDS if num_count == 1 else MULTI_DIGIT_HOLD_SECONDS
        elapsed_time = time.time() - candidate_start_time
        
        if elapsed_time >= hold_time:
            # 满足时间，且与上次发送的内容不同，则发送
            if current_str != last_sent_value:
                # 使用新的协议创建并发送数据帧
                frame = create_data_frame(current_str)
                serial.write(frame)
                last_sent_value = current_str
            # 发送后重置，避免在下一帧满足条件时重复发送
            candidate_str = None


while not app.need_exit():
    img = cam.read()
    if img is None:
        continue
    
    objs = detector.detect(img, conf_th=0.5, iou_th=0.45)

    # 新增：基于“屏幕中不可能出现相同数字”的规则，对检测结果进行预处理
    # 如果同一数字被多次检测，只保留置信度最高的一个
    unique_objs = {}
    for obj in objs:
        class_id = obj.class_id
        if class_id not in unique_objs or obj.score > unique_objs[class_id].score:
            unique_objs[class_id] = obj
    
    # 使用去重后的结果进行后续处理
    filtered_objs = list(unique_objs.values())

    # 提取当前帧的检测结果
    current_numbers.clear()
    for obj in filtered_objs:
        current_numbers.append({
            'label': detector.labels[obj.class_id],
            'x': float(obj.x), 'y': float(obj.y), 'w': float(obj.w), 'h': float(obj.h),
            'score': obj.score
        })

    # 更新并获取平滑后的追踪对象
    tracked_objects = update_tracked_objects(current_numbers, tracked_objects)
    
    # 在图像上绘制平滑后的边界框
    display_numbers = []
    for obj in tracked_objects:
        # 只显示近期更新过的对象，避免显示已经消失但仍在等待移除的对象
        if obj['unseen_frames'] == 0:
            x, y, w, h = int(obj['x']), int(obj['y']), int(obj['w']), int(obj['h'])
            img.draw_rect(x, y, w, h, color=image.COLOR_RED)
            msg = f'{obj["label"]}: {obj["score"]:.2f}'
            img.draw_string(x, y, msg, color=image.COLOR_RED)
            display_numbers.append(obj)

    # 处理并发送数据
    process_and_send_data(display_numbers)

    disp.show(img)
