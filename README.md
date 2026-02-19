# Bellek Kısıtlı Gömülü Sistemler İçin Dinamik Tensör (Dynamic Tensor) Gerçeklemesi

## 📌 Projenin Amacı

TinyML ve gömülü yapay zeka uygulamalarında Arduino ve ESP32 gibi mikrodenetleyiciler çok sınırlı RAM kapasitesine sahiptir (genellikle 320KB – 520KB).

Standart 32-bit float dizileri kullanıldığında bellek hızla tükenir.
Bu nedenle yapay zeka modellerinde **quantization (nicemleme)** ve düşük hassasiyetli veri tipleri kullanılır.

Bu projede C dili kullanılarak aşağıdaki veri tiplerini **tek bir tensör yapısı içinde dinamik olarak saklayabilen** düşük seviyeli bir tensör yapısı tasarlanmıştır:

* Float32 (yüksek hassasiyet)
* Float16 (orta hassasiyet)
* Int8 Quantized (düşük hassasiyet – bellek optimize)

Amaç, Python kütüphanelerindeki (TensorFlow / PyTorch) tensör mantığının gömülü sistemlerdeki primitive karşılığını oluşturmaktır.

---

## 🧠 Tensör Nedir?

Tensör, makine öğrenmesi modellerinde kullanılan çok boyutlu veri tutucudur.

Kullanım alanları:

* Giriş verileri
* Ağırlıklar (weights)
* Özellik haritaları (feature maps)
* Ara katman çıktıları

Üst seviye dillerde soyut bir veri yapısıdır.
Gömülü sistemlerde ise doğrudan bellek yönetimi ile manuel olarak oluşturulmalıdır.

Bu projede C dili kullanılarak ilkel (primitive) bir tensör gerçekleştirilmiştir.

---

## 🏗️ Sistem Mimarisi

### Dinamik Tensör Yapısı

```c
typedef struct {
    TensorType type;
    TensorData data;
    int size;
    float scale;
    int zero_point;
} DynamicTensor;
```

Tensör çalışma zamanında veri tipini değiştirebilir.

### Union ile Bellek Paylaşımı

```c
typedef union {
    float*    f32_ptr;
    uint16_t* f16_ptr;
    int8_t*   i8_ptr;
} TensorData;
```

Aynı anda yalnızca tek veri tipi bellekte yer kaplar → RAM tasarrufu sağlanır.

---

## ⚙️ Quantization (Nicemleme)

### Quantization Nedir?

Float değerlerin integer formata dönüştürülmesidir.

[
q = round(float / scale) + zero_point
]

Bu yöntem TinyML modellerinde:

* RAM kullanımını azaltır
* Flash boyutunu küçültür
* İşlemci yükünü azaltır

### Uygulanan İşlemler

* Min-Max kalibrasyonu
* Scale hesaplama
* Zero-Point hesaplama
* INT8 sınırlandırma (-128, 127)
* Dequantization hata analizi

---

## 🔬 Float16 Simülasyonu

Masaüstü C derleyicilerinde native float16 desteği bulunmadığından simülasyon yapılmıştır.

Bu sayede hassasiyet ve bellek kullanımı arasındaki fark gözlemlenmiştir:

Float32 > Float16 > Int8

---

## 💾 Bellek Karşılaştırması

| Veri Tipi | Eleman Başına Bellek |
| --------- | -------------------- |
| Float32   | 4 Byte               |
| Float16   | 2 Byte               |
| Int8      | 1 Byte               |

Quantization sonrası yaklaşık **%75 bellek tasarrufu** elde edilir.

---

## 🧪 Program Çalışma Aşamaları

1. Sensör verisi Float32 olarak oluşturulur
2. INT8 quantization uygulanır
3. Dequantization ile hata oranı ölçülür
4. Float16 simülasyonu yapılır
5. Bellek kullanımı karşılaştırılır

---

## 🐞 Debugger ile İnceleme

Debugger kullanılarak:

* Aynı tensörün farklı veri tiplerinde nasıl yorumlandığı
* Bellek adreslerinin değişmediği
* Veri temsilinin değiştiği

gözlemlenebilir.

---

## 🧩 Kullanılan Kavramlar

* Struct ve Union
* Dinamik bellek yönetimi (malloc/free)
* Tip dönüşümü
* Quantization
* Dequantization
* Bellek optimizasyonu

---



## 📊 Sonuç

Bu projede gömülü sistemlerde yapay zeka modellerinin çalıştırılabilmesi için gerekli olan:

* Bellek optimizasyonu
* Veri tipi esnekliği
* Quantization mantığı

C dili seviyesinde başarıyla gerçekleştirilmiştir.

Bu yapı TinyML uygulamalarında tensör yönetiminin temel prensibini temsil eder.

