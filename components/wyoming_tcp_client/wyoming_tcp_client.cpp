#include "wyoming_tcp_client.h"
#include "esphome/core/log.h"

namespace esphome {
namespace wyoming_tcp_client {

static const char *const TAG = "wyoming_tcp";

void WyomingTcpClient::setup() {
  ESP_LOGI(TAG, "Setting up Wyoming TCP client -> %s:%d",
           this->host_.c_str(), this->port_);

  this->mic_buffer_ = RingBuffer::create(16384);  // ~500ms at 16kHz 16-bit
  this->spk_buffer_ = RingBuffer::create(32768);  // ~1s response buffer

  // Register microphone callback
  this->mic_source_->add_data_callback(
      [this](const std::vector<uint8_t> &data) {
        if (this->state_ == State::STREAMING) {
          this->mic_buffer_->write((void *) data.data(), data.size());
        }
      });

  ESP_LOGI(TAG, "Ready, waiting for wake word");
}

void WyomingTcpClient::loop() {
  // Play any received audio from speaker buffer
  if (this->state_ == State::RECEIVING ||
      this->spk_buffer_->available() > 0) {
    uint8_t buf[1024];
    size_t available = this->spk_buffer_->available();
    while (available > 0) {
      size_t to_read = std::min(available, sizeof(buf));
      this->spk_buffer_->read((void *) buf, to_read, 0);
      this->speaker_->play(buf, to_read);
      available = this->spk_buffer_->available();
    }
  }
}

void WyomingTcpClient::start() {
  if (this->state_ != State::IDLE) {
    ESP_LOGW(TAG, "Already active, ignoring start");
    return;
  }

  ESP_LOGI(TAG, "Starting session");
  this->state_ = State::CONNECTING;

  // Launch network task on Core 1
  xTaskCreatePinnedToCore(WyomingTcpClient::net_task_, "wyoming_net",
                          8192, this, 5, &this->net_task_handle_, 1);
}

void WyomingTcpClient::stop() {
  ESP_LOGI(TAG, "Stopping session");
  this->state_ = State::IDLE;
  this->disconnect_();
}

// Stubs — implemented in Task 2 and 3
bool WyomingTcpClient::connect_() { return false; }
void WyomingTcpClient::disconnect_() {}
bool WyomingTcpClient::send_event_(const char *, const char *,
                                    const uint8_t *, size_t) { return false; }
void WyomingTcpClient::send_audio_start_() {}
void WyomingTcpClient::send_audio_stop_() {}
bool WyomingTcpClient::receive_events_() { return false; }
void WyomingTcpClient::net_task_(void *param) {}
void WyomingTcpClient::net_task_loop_() {}
void WyomingTcpClient::handle_received_event_(const std::string &,
    const std::string &, const std::vector<uint8_t> &) {}

}  // namespace wyoming_tcp_client
}  // namespace esphome
