fs = 48000; % sample rate
f1 = 250;
f2 = 2000;
f0 = (f2 - f1)/2 + f1;
w1 = (f1 / (fs / 2)) * pi;
w2 = (f2 / (fs / 2)) * pi;
w0 = (f0 / (fs / 2)) * pi; % centre frequency for bandpass filter
wb = w2 - w1; % bandwidth of bandpass filter
wc = wb / 2; % cutoff frequency for needed lowpass filter
N = 700; % number of filter coefficients
win = hamming(N)'; % window: transposed so .* can be used
n = 0:N-1;
L = (N-1)/2;
hLP_tmp = win .* ((wc/pi) * sinc((wc/pi) * (n - L)));
hBP = 2 * cos((n - L) * w0) .* hLP_tmp;
figure; stem(n, hBP);
figure; freqz(hBP, 1);
subplot(2,1,1); refline(0,-3); % Add a -3 dB reference line
figure; zplane(hBP, 1);